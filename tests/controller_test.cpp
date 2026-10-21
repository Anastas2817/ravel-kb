// Тесты Controller (app layer). У Controller нет read-API — observable
// surface это протокол команд (expected с кодом ошибки) и файловые эффекты
// Open/Save/SaveAs. Проверяем только это; состояние графа покрывают
// graph_test (ядро) и property_test (оракулы).
//
// Файловые тесты работают в temp_directory_path() с уникальными именами
// и подчищают за собой. SaveAs атомарен (temp + rename) — его корректность
// проверяем косвенно: записанный файл должен читаться JsonStorage::Load.

#include "controller.hpp"

#include "storage.hpp"

#include <filesystem>
#include <fstream>
#include <stdexcept>

#include <gtest/gtest.h>

namespace {

namespace fs = std::filesystem;

// Фикстура: уникальные пути во временном каталоге + подчистка (включая
// возможный недоперемещённый .tmp — след атомарного SaveAs при сбоях).
class ControllerIoTest : public ::testing::Test {
 protected:
  void TearDown() override {
    std::error_code ec;
    fs::remove(good_path_, ec);
    fs::remove(good_path_.string() + ".tmp", ec);
    fs::remove(source_path_, ec);
  }

  fs::path good_path_ = fs::temp_directory_path() / "ravel_ctrl_good.json";
  fs::path source_path_ = fs::temp_directory_path() / "ravel_ctrl_source.json";
};

// Готовит файл с графом, в котором есть онтология (тип "nt"): контроллеру
// нужен такой файл, чтобы команды добавления проходили успешно — своего
// API для AddNodeType у Controller нет (это намеренно: онтология в v1
// задаётся загруженным файлом).
void WriteGraphWithOntology(const fs::path& p) {
  ravel::Graph g;
  g.AddNodeType("nt", "#112233", "rect", "#445566");
  g.AddRelationType("rt", "#112233", "solid", false, true);
  std::ofstream out(p, std::ios::binary);
  ravel::JsonStorage::Save(g.ToSnapshot(), out);
}

TEST_F(ControllerIoTest, OpenMissingFileReturnsNoPath) {
  ravel::app::Controller c;
  const auto res = c.Open(good_path_);
  ASSERT_FALSE(res.has_value());
  EXPECT_EQ(res.error(), ravel::app::ControllerError::NoPath);
}

TEST_F(ControllerIoTest, OpenGarbageFileReturnsIoError) {
  {
    std::ofstream out(good_path_, std::ios::binary);
    out << "this is not json {{{";
  }
  ravel::app::Controller c;
  const auto res = c.Open(good_path_);
  ASSERT_FALSE(res.has_value());
  EXPECT_EQ(res.error(), ravel::app::ControllerError::IoError);
}

TEST(ControllerSession, FreshControllerHasNothingToUndo) {
  ravel::app::Controller c;
  const auto res = c.Undo();
  ASSERT_FALSE(res.has_value());
  EXPECT_EQ(res.error(), ravel::app::CommandError::NothingToUndo);
}

TEST(ControllerSession, FreshControllerHasNothingToRedo) {
  ravel::app::Controller c;
  const auto res = c.Redo();
  ASSERT_FALSE(res.has_value());
  EXPECT_EQ(res.error(), ravel::app::CommandError::NothingToRedo);
}

TEST(ControllerSession, SaveWithoutPathReturnsNoPath) {
  ravel::app::Controller c;
  const auto res = c.Save();
  ASSERT_FALSE(res.has_value());
  EXPECT_EQ(res.error(), ravel::app::ControllerError::NoPath);
}

// Текущий контракт доменных проверок ядра — исключения (переход на
// expected зафиксирован в roadmap как направление, не как этот тест).
// Фиксируем поведение как регрессионную границу: ошибка не должна
// проглатываться и не должна портить сессию.
TEST(ControllerSession, AddNodeWithUnknownTypeThrowsThrough) {
  ravel::app::Controller c;
  EXPECT_THROW((void)c.AddNode("n", "missing"), std::invalid_argument);
}

TEST_F(ControllerIoTest, SaveAsWritesFileReadableByStorage) {
  ravel::app::Controller c;
  const auto res = c.SaveAs(good_path_);
  ASSERT_TRUE(res.has_value()) << "error=" << (res ? -1 : static_cast<int>(res.error()));
  std::ifstream in(good_path_, std::ios::binary);
  ASSERT_TRUE(in.good()) << "SaveAs не оставил файла по целевому пути";
  // Атомарность косвенно: файл цел, читается и парсится.
  EXPECT_NO_THROW((void)ravel::JsonStorage::Load(in));
}

TEST_F(ControllerIoTest, OpenThenSaveWithoutExplicitPath) {
  WriteGraphWithOntology(source_path_);
  ravel::app::Controller c;
  ASSERT_TRUE(c.Open(source_path_).has_value());
  // Save после Open использует путь, полученный из Open.
  ASSERT_TRUE(c.Save().has_value());
}

TEST_F(ControllerIoTest, SaveAsToMissingDirectoryReturnsIoError) {
  ravel::app::Controller c;
  const fs::path no_such_dir = fs::temp_directory_path() / "ravel_ctrl_no_such_dir" / "f.json";
  const auto res = c.SaveAs(no_such_dir);
  ASSERT_FALSE(res.has_value());
  EXPECT_EQ(res.error(), ravel::app::ControllerError::IoError);
  std::error_code ec;
  ASSERT_FALSE(fs::exists(no_such_dir, ec));
}

// Протокол команд поверх загруженной сессии: Execute → Undo → Redo →
// Undo, затем дно стека. Open сбрасывает летопись (свежая сессия — один
// undo после одного AddNode, второй — NothingToUndo).
TEST_F(ControllerIoTest, CommandFlowAfterOpen) {
  WriteGraphWithOntology(source_path_);
  ravel::app::Controller c;
  const auto open_res = c.Open(source_path_);
  ASSERT_TRUE(open_res.has_value())
      << "open error=" << (open_res ? -1 : static_cast<int>(open_res.error()));
  ASSERT_TRUE(c.AddNode("n", "nt").has_value());
  ASSERT_TRUE(c.AddRelation(1, 1, "rt").has_value());
  ASSERT_TRUE(c.DeleteRelations({1}).has_value());
  ASSERT_TRUE(c.Undo().has_value());  // delete отменён
  ASSERT_TRUE(c.Undo().has_value());  // relation убран
  ASSERT_TRUE(c.Redo().has_value());  // relation восстановлен (restore-путь)
  ASSERT_TRUE(c.Undo().has_value());
  ASSERT_TRUE(c.Undo().has_value());  // AddNode убран
  const auto res = c.Undo();
  ASSERT_FALSE(res.has_value());
  EXPECT_EQ(res.error(), ravel::app::CommandError::NothingToUndo);
}

}  // namespace

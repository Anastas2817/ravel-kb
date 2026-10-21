#include "controller.hpp"

#include <filesystem>

#include <gtest/gtest.h>

namespace {

// Золотой файл: examples/example.ravel.json — эталонный сценарий «граф
// только с типами» (регрессионная граница бага пустого снапшота в
// FromSnapshot). Тест держит пример открываемым: сломался формат или
// словарь значений (shape/arrow/цвет) — падает здесь, а не в руках юзера.
TEST(ExampleFile, OpensViaController) {
  const std::filesystem::path p =
      std::filesystem::path(RAVEL_REPO_ROOT) / "examples" / "example.ravel.json";
  ravel::app::Controller c;
  const auto res = c.Open(p);
  ASSERT_TRUE(res.has_value()) << "error=" << (res.has_value() ? -1 : static_cast<int>(res.error()))
                               << " path=" << p;
}

}  // namespace

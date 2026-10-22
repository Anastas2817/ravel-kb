#include "controller.hpp"

#include <fstream>

namespace ravel::app {

ControllerResult Controller::Open(const std::filesystem::path& p) {
  std::ifstream file(p, std::ios::binary);
  if (!file) {
    return std::unexpected(ControllerError::NoPath);
  }

  SessionState next;

  try {
    next.g = ravel::Graph::FromSnapshot(ravel::JsonStorage::Load(file));
    next.chronicle = ravel::app::CommandChronicle{};
    next.file_path = p;
    next.saved_bookmark_ = 0;
  } catch (const std::exception&) {
    return std::unexpected(ControllerError::IoError);
  }

  s_ = std::move(next);
  return {};
}

ControllerResult Controller::Save() {
  if (s_.file_path == std::nullopt) {
    return std::unexpected(ControllerError::NoPath);
  }

  return SaveAs(s_.file_path.value());
}

ControllerResult Controller::SaveAs(const std::filesystem::path& p) {
  std::filesystem::path temp = p;
  temp += ".tmp";

  std::ofstream file(temp, std::ios::binary);
  if (!file) {
    return std::unexpected(ControllerError::IoError);
  }

  try {
    ravel::JsonStorage::Save(s_.g.ToSnapshot(), file);
    if (!file) {
      throw std::runtime_error("close failed");
    }
  } catch (const std::exception&) {
    file.close();
    std::error_code ec;
    std::filesystem::remove(temp, ec);
    return std::unexpected(ControllerError::IoError);
  }

  file.close();
  std::error_code ec;
  std::filesystem::rename(temp, p, ec);
  if (ec) {
    std::filesystem::remove(temp, ec);
    return std::unexpected(ControllerError::IoError);
  }

  s_.file_path = p;
  s_.saved_bookmark_ = s_.chronicle.bookmark();
  return {};
}

CommandResult Controller::AddNode(std::string title, std::string type, std::string description,
                                  double size) {
  const ravel::app::AddNodeCmd cmd{.title = std::move(title),
                                   .type = std::move(type),
                                   .description = std::move(description),
                                   .size = size};
  return s_.chronicle.Execute(cmd, s_.g);
}

CommandResult Controller::AddRelation(ravel::NodeId from, ravel::NodeId to, std::string type,
                                      std::string description) {
  const ravel::app::AddRelationCmd cmd{
      .from = from, .to = to, .type = std::move(type), .description = std::move(description)};
  return s_.chronicle.Execute(cmd, s_.g);
}

CommandResult Controller::DeleteNodes(std::vector<ravel::NodeId> nodes) {
  const ravel::app::DeleteNodesCmd cmd{std::move(nodes), {}};
  return s_.chronicle.Execute(cmd, s_.g);
}

CommandResult Controller::DeleteRelations(std::vector<ravel::RelationId> relations) {
  const ravel::app::DeleteRelationsCmd cmd{.ids = std::move(relations)};
  return s_.chronicle.Execute(cmd, s_.g);
}

CommandResult Controller::Undo() {
  return s_.chronicle.Undo(s_.g);
}

CommandResult Controller::Redo() {
  return s_.chronicle.Redo(s_.g);
}

}  // namespace ravel::app

#ifndef RAVEL_APP_INCLUDE_CONTROLLER_HPP_
#define RAVEL_APP_INCLUDE_CONTROLLER_HPP_

#include "command.hpp"
#include "storage.hpp"

#include <filesystem>
#include <optional>

namespace ravel::app {

enum class ControllerError { NoPath, IoError, NothingToUndo, NothingToRedo };
using ControllerResult = std::expected<void, ControllerError>;

class Controller {
 public:
  ControllerResult Open(const std::filesystem::path& p);
  ControllerResult Save();
  ControllerResult SaveAs(const std::filesystem::path& p);

  CommandResult AddNode(std::string title, std::string type, std::string description = "",
                        double size = 1);
  CommandResult AddRelation(ravel::NodeId from, ravel::NodeId to, std::string type,
                            std::string description = "");
  CommandResult DeleteNodes(std::vector<ravel::NodeId> nodes);
  CommandResult DeleteRelations(std::vector<ravel::RelationId> relations);
  CommandResult Undo();
  CommandResult Redo();

 private:
  ravel::JsonStorage storage_;

  struct SessionState {
    ravel::Graph g;
    ravel::app::CommandChronicle chronicle;
    std::optional<std::filesystem::path> file_path = std::nullopt;

    std::size_t saved_bookmark_ = 0;
  };

  SessionState s_;
};

}  // namespace ravel::app

#endif  // RAVEL_APP_INCLUDE_CONTROLLER_HPP_

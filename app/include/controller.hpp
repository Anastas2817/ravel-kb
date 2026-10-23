#ifndef RAVEL_APP_INCLUDE_CONTROLLER_HPP_
#define RAVEL_APP_INCLUDE_CONTROLLER_HPP_

#include "command.hpp"
#include "storage.hpp"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace ravel::app {

enum class ControllerError : std::uint8_t { NoPath, IoError, NothingToUndo, NothingToRedo };
using ControllerResult = std::expected<void, ControllerError>;

struct NodeView {  // DTO
  ravel::NodeId id;
  std::string title;
  std::string description;
  double size = 0;
  std::optional<ravel::Position> position;

  std::string type;
  std::string color;
  std::string shape;
  std::string frame_color;
};

class Controller {
 public:
  ControllerResult Open(const std::filesystem::path& p);
  ControllerResult Save();
  ControllerResult SaveAs(const std::filesystem::path& p);

  CommandResult AddNode(std::string title, std::string type, std::string description = "",
                        double size = 1, std::optional<ravel::Position> position = std::nullopt);
  CommandResult AddRelation(ravel::NodeId from, ravel::NodeId to, std::string type,
                            std::string description = "");
  CommandResult DeleteNodes(std::vector<ravel::NodeId> nodes);
  CommandResult DeleteRelations(std::vector<ravel::RelationId> relations);
  CommandResult Undo();
  CommandResult Redo();
  CommandResult MoveNode(ravel::NodeId id, ravel::Position to);

  std::vector<NodeView> Nodes() const;
  std::vector<std::string> NodeTypeNames() const;

 private:
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

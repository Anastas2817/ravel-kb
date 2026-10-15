#ifndef RAVEL_APP_INCLUDE_COMMAND_HPP_
#define RAVEL_APP_INCLUDE_COMMAND_HPP_

#include "graph.hpp"

#include <expected>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace ravel::app {

enum class CommandError {
  UnknownType,
  EntityGone,
  QuotaExceeded,
  /* Io — later */
};

using CommandResult = std::expected<void, CommandError>;

// Redo path: obtained_id != 0 means this command has been executed and
// undone before; re-executing restores the exact same entity (soft-delete,
// ADR-002), it does NOT create a new one. Calling Do twice without an
// intervening Undo is a stack-contract violation, caught by the Restore*
// debug asserts.
struct AddNodeCmd {
  std::string title;
  std::string type;
  std::string description;
  double size = 1;
  ravel::NodeId obtained_id = 0;
  CommandResult Do(ravel::Graph& g) {
    if (obtained_id == 0) {
      obtained_id = g.AddNode(title, type, description, size);
    } else {
      g.RestoreNodes({obtained_id});
    }
    return {};
  }
  CommandResult Undo(ravel::Graph& g) {
    g.DeleteNodes({obtained_id});
    return {};
  }
};

// Same redo contract as AddNodeCmd: obtained_id != 0 => restore, not create.
struct AddRelationCmd {
  ravel::NodeId from = 0;
  ravel::NodeId to = 0;
  std::string type;
  std::string description;
  ravel::RelationId obtained_id = 0;
  CommandResult Do(ravel::Graph& g) {
    if (obtained_id == 0) {
      obtained_id = g.AddRelation(from, to, type, description);
    } else {
      g.RestoreRelations({obtained_id});
    }
    return {};
  }
  CommandResult Undo(ravel::Graph& g) {
    g.DeleteRelations({obtained_id});
    return {};
  }
};

struct DeleteNodesCmd {
  std::vector<ravel::NodeId> nodes;
  std::vector<ravel::RelationId> deleted_relations;
  CommandResult Do(ravel::Graph& g) {
    g.DeleteNodes(nodes);
    return {};
  }
  CommandResult Undo(ravel::Graph& g) {
    g.RestoreNodes(nodes);
    g.RestoreRelations(deleted_relations);
    return {};
  }
};

struct DeleteRelationsCmd {
  std::vector<ravel::RelationId> ids;
  CommandResult Do(ravel::Graph& g) {
    g.DeleteRelations(ids);
    return {};
  }
  CommandResult Undo(ravel::Graph& g) {
    g.RestoreRelations(ids);
    return {};
  }
};

// Precondition: the node already has a position. The first (computed)
// placement by layout is initialization, not a user action, and bypasses
// the undo stack — hence .value(), which throws on violation. See
// implementation.md R14.
struct MoveNodeCmd {
  ravel::NodeId id;
  std::pair<double, double> from;
  std::pair<double, double> to;
  CommandResult Do(ravel::Graph& g) {
    from = g.MoveNode(id, to).value();
    return {};
  }
  CommandResult Undo(ravel::Graph& g) {
    g.MoveNode(id, from);
    return {};
  }
};

template <typename C>
concept Command = requires(C& c, ravel::Graph& g) {
  { c.Do(g) } -> std::same_as<CommandResult>;
  { c.Undo(g) } -> std::same_as<CommandResult>;
};
static_assert(Command<AddNodeCmd> && Command<AddRelationCmd> && Command<DeleteNodesCmd> &&
              Command<DeleteRelationsCmd> && Command<MoveNodeCmd>);

using AnyCommand =
    std::variant<AddNodeCmd, AddRelationCmd, DeleteNodesCmd, DeleteRelationsCmd, MoveNodeCmd>;

inline void Do(AnyCommand& c, ravel::Graph& g) {
  std::visit([&g](auto& x) { x.Do(g); }, c);
}
inline void Undo(AnyCommand& c, ravel::Graph& g) {
  std::visit([&g](auto& x) { x.Undo(g); }, c);
}

}  // namespace ravel::app

#endif  // RAVEL_APP_INCLUDE_COMMAND_HPP_

#ifndef RAVEL_APP_INCLUDE_COMMAND_HPP_
#define RAVEL_APP_INCLUDE_COMMAND_HPP_

#include "graph.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace ravel::app {

enum class CommandError : std::uint8_t {
  UnknownType,
  EntityGone,
  QuotaExceeded,
  NothingToUndo,
  NothingToRedo
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
  std::vector<ravel::NodeId> ids;
  std::vector<ravel::RelationId> deleted_relations;
  CommandResult Do(ravel::Graph& g) {
    deleted_relations = g.DeleteNodes(ids);
    return {};
  }
  CommandResult Undo(ravel::Graph& g) const {
    g.RestoreNodes(ids);
    g.RestoreRelations(deleted_relations);
    return {};
  }
};

struct DeleteRelationsCmd {
  std::vector<ravel::RelationId> ids;
  CommandResult Do(ravel::Graph& g) const {
    g.DeleteRelations(ids);
    return {};
  }
  CommandResult Undo(ravel::Graph& g) const {
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
  Position from;
  Position to;
  CommandResult Do(ravel::Graph& g) {
    // .value() is deliberate: see the struct comment above (R14).
    from = g.MoveNode(id, to).value();  // NOLINT(bugprone-unchecked-optional-access)
    return {};
  }
  CommandResult Undo(ravel::Graph& g) const {
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

inline CommandResult Do(AnyCommand& c, ravel::Graph& g) {
  return std::visit([&g](auto& x) { return x.Do(g); }, c);
}
inline CommandResult Undo(AnyCommand& c, ravel::Graph& g) {
  return std::visit([&g](auto& x) { return x.Undo(g); }, c);
}

// The only point of state change in Graph.
class CommandChronicle {
 public:
  std::size_t bookmark() const noexcept {
    return bookmark_;
  }
  template <Command C>
  CommandResult Execute(C cmd, ravel::Graph& g) {
    CommandResult res = cmd.Do(g);
    if (res) {
      // NOLINTNEXTLINE(bugprone-narrowing-conversions): bookmark_ <=
      // chronicle_.size() (class invariant), so the size_t fits ptrdiff_t on
      // any platform where a vector of this size could exist.
      chronicle_.erase(chronicle_.begin() + static_cast<std::ptrdiff_t>(bookmark_),
                       chronicle_.end());
      chronicle_.emplace_back(std::move(cmd));
      ++bookmark_;
    }
    return res;
  }

  CommandResult Undo(ravel::Graph& g) {
    if (bookmark_ == 0) {
      return std::unexpected(CommandError::NothingToUndo);
    }
    CommandResult res = ravel::app::Undo(chronicle_[bookmark_ - 1], g);
    if (res) {
      --bookmark_;
    }
    return res;
  }

  CommandResult Redo(ravel::Graph& g) {
    if (bookmark_ >= chronicle_.size()) {
      return std::unexpected(CommandError::NothingToRedo);
    }
    CommandResult res = ravel::app::Do(chronicle_[bookmark_], g);
    if (res) {
      ++bookmark_;
    }
    return res;
  }

 private:
  std::vector<AnyCommand> chronicle_;
  std::size_t bookmark_ = 0;  // invariant : 0 <= bookmark <= chronicle.size()
};

}  // namespace ravel::app

#endif  // RAVEL_APP_INCLUDE_COMMAND_HPP_

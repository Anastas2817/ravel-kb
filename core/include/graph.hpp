#ifndef RAVEL_CORE_INCLUDE_GRAPH_HPP_
#define RAVEL_CORE_INCLUDE_GRAPH_HPP_

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <map>
#include <optional>
#include <ranges>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ravel {

using NodeId = std::uint64_t;
using RelationId = std::uint64_t;

[[nodiscard]] constexpr bool IsHexDigit(char c) noexcept {
  return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}
[[nodiscard]] constexpr bool IsValidHexColor(std::string_view color) noexcept {
  return color.size() == 7 && color[0] == '#' &&
         std::all_of(color.begin() + 1, color.end(), IsHexDigit);
}
[[nodiscard]] constexpr bool IsValidShape(std::string_view shape) {
  return ((shape == "rect") || (shape == "oval"));
}

class NodeType {
 public:
  NodeType(std::string color, std::string shape, std::string frame_color)
      : color_(IsValidHexColor(color)
                   ? color
                   : throw std::invalid_argument(
                         "NodeType::NodeType tried to create NodeType with non-valid color")),
        shape_(IsValidShape(shape) ? shape
                                   : throw std::invalid_argument(
                                         "NodeType tried to create NodeType with non-valid shape")),
        frame_color_(
            IsValidHexColor(frame_color)
                ? frame_color
                : throw std::invalid_argument(
                      "NodeType::NodeType tried to create NodeType with non-valid frame_color")) {
  }

  [[nodiscard]] const std::string& color() const noexcept {
    return color_;
  }
  [[nodiscard]] const std::string& shape() const noexcept {
    return shape_;
  }
  [[nodiscard]] const std::string& frame_color() const noexcept {
    return frame_color_;
  }
  bool operator==(const NodeType& other) const = default;

 private:
  std::string color_ = "#000000";
  std::string shape_ = "rect";
  std::string frame_color_ = "#000000";
};

class Node {  // DTO
 public:
  Node(NodeId id, std::string title, std::string type, std::string description = "",
       double size = 1)
      : id_(id),
        type_(std::move(type)),
        title_(std::move(title)),
        description_(std::move(description)),
        size_((size > 0 && std::isfinite(size))
                  ? size
                  : throw std::invalid_argument(
                        "Node::Node tried to create Node with non-positive size")) {
  }

  [[nodiscard]] NodeId id() const noexcept {
    return id_;
  }
  [[nodiscard]] const std::string& type() const noexcept {
    return type_;
  }

  [[nodiscard]] const std::string& title() const noexcept {
    return title_;
  }
  [[nodiscard]] const std::string& description() const noexcept {
    return description_;
  }
  [[nodiscard]] double size() const noexcept {
    return size_;
  }

  // Exact == for doubles is intentional: size_ is stored
  // values compared for identity after serialization round-trips, not results
  // of floating-point computation. A bit-level mismatch here is a storage bug
  // and must surface, not be masked by an epsilon. Approximate comparison is
  // for computed values (layout geometry) only. See implementation.md,
  // note to rule 18.
  bool operator==(const Node& other) const = default;

 private:
  // Identity
  NodeId id_ = 0;
  std::string type_;

  // Local data
  std::string title_;
  std::string description_;
  double size_ = 1;
};

[[nodiscard]] constexpr bool IsValidArrow(std::string_view arrow) {
  return (arrow == "solid" || arrow == "dashed" || arrow == "dotted");
}

template <typename Fn>
concept LiveNodeCallback = std::invocable<Fn&, const Node&> &&
                           std::convertible_to<std::invoke_result_t<Fn&, const Node&>, bool>;

class RelationType {
 public:
  RelationType(std::string color, std::string arrow, bool symmetric, bool transitive)
      : color_(IsValidHexColor(color)
                   ? color
                   : throw std::invalid_argument("RelationType::RelationType tried to create "
                                                 "RelationType with non-valid color")),
        arrow_(IsValidArrow(arrow)
                   ? arrow
                   : throw std::invalid_argument("RelationType::RelationType tried to create "
                                                 "RelationType with non-valid arrow")),
        symmetric_(symmetric),
        transitive_(transitive) {
  }

  [[nodiscard]] const std::string& color() const noexcept {
    return color_;
  }
  [[nodiscard]] const std::string& arrow() const noexcept {
    return arrow_;
  }
  [[nodiscard]] bool symmetric() const noexcept {
    return symmetric_;
  }
  [[nodiscard]] bool transitive() const noexcept {
    return transitive_;
  }
  bool operator==(const RelationType& other) const = default;

 private:
  std::string color_ = "#000000";
  std::string arrow_ = "solid";  // solid, dashed, dotted
  bool symmetric_ = false;
  bool transitive_ = false;
};

class Relation {  // DTO
 public:
  Relation(RelationId id, NodeId from, NodeId to, std::string type, std::string description = "")
      : id_(id),
        from_(from),
        to_(to),
        type_(std::move(type)),
        description_(std::move(description)) {
  }

  [[nodiscard]] RelationId id() const noexcept {
    return id_;
  }
  [[nodiscard]] NodeId from() const noexcept {
    return from_;
  }
  [[nodiscard]] NodeId to() const noexcept {
    return to_;
  }
  [[nodiscard]] const std::string& type() const noexcept {
    return type_;
  }

  [[nodiscard]] const std::string& description() const noexcept {
    return description_;
  }

  bool operator==(const Relation& other) const = default;

 private:
  RelationId id_ = 0;
  NodeId from_ = 0;
  NodeId to_ = 0;
  std::string type_;

  std::string description_;
};

template <typename Fn>
concept LiveRelationCallback =
    std::invocable<Fn&, const Relation&> &&
    std::convertible_to<std::invoke_result_t<Fn&, const Relation&>, bool>;

struct Position {
  double x_;
  double y_;

  bool operator==(const Position&) const =
      default;  // Exact == for doubles is intentional: x_, y_ are stored
  // values compared for identity after serialization round-trips, not results
  // of floating-point computation. A bit-level mismatch here is a storage bug
  // and must surface, not be masked by an epsilon. Approximate comparison is
  // for computed values (layout geometry) only. See implementation.md,
  // note to rule 18.
};

struct GraphSnapshot {
  std::unordered_map<std::string, NodeType> ontology_node;
  std::unordered_map<std::string, RelationType> ontology_relation;

  std::vector<Node> nodes;
  std::vector<Relation> relations;
  std::vector<std::optional<Position>> positions;

  NodeId next_id_node = 1;
  RelationId next_id_relation = 1;
};

class Graph {
 public:
  [[nodiscard]] NodeId next_id_node() const noexcept {
    return next_id_node_;
  }
  [[nodiscard]] NodeId next_id_relation() const noexcept {
    return next_id_relation_;
  }
  [[nodiscard]] const std::unordered_map<std::string, NodeType>& ontology_node()
      const noexcept {  /// @return Const
    /// reference to the type dictionary. Lifetime is bound to *this; do not store the reference
    /// beyond the Graph's lifetime.
    return ontology_node_;
  }
  [[nodiscard]] const std::unordered_map<std::string, RelationType>& ontology_relation()
      const noexcept {  /// @return
    /// Const reference to the type dictionary. Lifetime is bound to *this; do not store the
    /// reference beyond the Graph's lifetime.
    return ontology_relation_;
  }

  void AddNodeType(std::string name, std::string color, std::string shape,
                   std::string frame_color) {
    if (ontology_node_.contains(name)) {
      throw std::invalid_argument("Graph::AddNodeType tried to add the same NodeType");
    }
    ontology_node_.try_emplace(
        name, NodeType(std::move(color), std::move(shape), std::move(frame_color)));
  }
  void AddRelationType(std::string name, std::string color, std::string arrow, bool symmetric,
                       bool transitive) {
    if (ontology_relation_.contains(name)) {
      throw std::invalid_argument("Graph::AddRelationType tried to add the same RelationType");
    }
    ontology_relation_.try_emplace(
        name, RelationType(std::move(color), std::move(arrow), symmetric, transitive));
  }
  [[nodiscard]] const NodeType& ResolveNode(const std::string& name) const {
    return ontology_node_.at(name);
  }
  [[nodiscard]] const RelationType& ResolveRelation(const std::string& name) const {
    return ontology_relation_.at(name);
  }

  [[nodiscard]] const Node& GetNode(NodeId id) const {
    if (!HasNode(id)) {
      throw std::invalid_argument("Graph::GetNode tried to access non-existent Node");
    }
    return nodes_[id - 1];
  }
  [[nodiscard]] std::optional<Position> GetPosition(NodeId id) const {
    if (!HasNode(id)) {
      throw std::invalid_argument("Graph::GetPosition tried to access non-existent Node");
    }
    return positions_[id - 1];
  }
  NodeId AddNode(std::string title, std::string type, std::string description = "",
                 double size = 1);
  [[nodiscard]] bool HasNode(NodeId id) const noexcept;
  RelationId AddRelation(NodeId from, NodeId to, std::string type, std::string description = "");
  [[nodiscard]] bool HasRelation(RelationId id) const noexcept;
  [[nodiscard]] const Relation& GetRelation(RelationId id) const {
    if (!HasRelation(id)) {
      throw std::invalid_argument("Graph::GetRelation tried to access non-existent Relation");
    }
    return relations_[id - 1];
  }
  // Composite convenience: AddNode + AddRelation(s) in one validated call.
  // A real UI scenario exists (create a node already linked), but the
  // composition itself does not need Graph's privacy — candidate for moving
  // to the App layer (built on public AddNode/AddRelation) once the UI
  // scenario lands. Until then it stays, covered by tests.
  NodeId AddNodeWithRelations(std::string title, const std::string& type,
                              const std::vector<std::pair<NodeId, std::string>>& froms,
                              const std::vector<std::pair<NodeId, std::string>>& tos,
                              std::string description = "", double size = 1);
  std::optional<Position> MoveNode(NodeId id, Position pos);
  // implementation.md («Контракт удаления») - idempotence for delete*

  std::vector<RelationId> DeleteNodes(const std::vector<NodeId>& ids);
  void DeleteRelations(const std::vector<RelationId>& ids);

  /**
   * @brief Ids of all relations incident to a live node (incoming and
   *        outgoing together; direction of a relation is recovered via
   *        GetRelation(rid)).
   * @param id id of a live node; std::invalid_argument otherwise.
   * @return Const reference into the adjacency index. Two lifetime limits,
   *         both contractual: the reference dies with *this, and it dies on
   *         ANY mutating call (Add/Delete/Restore/MoveNode) — never hold
   *         it across a mutation, obtain it again after one.
   * @note Internal id order is unspecified and unstable across mutations
   *       (implementation.md R3): rely on membership, not on positions.
   *       Holding an invalidated reference is UB — tests cannot catch it,
   *       the contract lives here and is enforced by review.
   */
  [[nodiscard]] const std::vector<RelationId>& GetIncidentRelations(NodeId id) const {
    if (!HasNode(id)) {
      throw std::invalid_argument("Graph::GetIncidentRelations tried to access non-existent Node");
    }
    return access_.at(id);
  }
  // Undo surface (ADR-002, consequence 12). Ownership invariant: called
  // ONLY from the undo stack (App layer) — never directly from UI. The
  // preconditions below are debug-asserts, not exceptions: a violation is
  // an undo-logic bug, not bad data. In Release (NDEBUG) a violated
  // precondition degrades to silent inconsistency — accepted deliberately
  // for the undo path, which is why the ownership invariant is written
  // here rather than left as a convention. Order: restore nodes BEFORE
  // their relations (foreign endpoints must be live).
  void RestoreNodes(std::vector<NodeId> ids);
  void RestoreRelations(std::vector<RelationId> ids);

  [[nodiscard]] std::set<NodeId> GetNeighbors(NodeId id) const;

  // The single sanctioned iteration point over live nodes, in ascending id
  // order. Callback contract: do not mutate the graph during iteration — UB
  // (ADR-002, "Decision"); mutations go through collect-then-apply. Returning
  // false stops the iteration early (early exit for algorithms).
  template <LiveNodeCallback Fn>
  void ForEachLiveNode(Fn&& fn) const {
    for (const NodeId id : access_ | std::views::keys) {
      if (!fn(nodes_[id - 1])) {
        return;
      }
    }
  }
  // Same for relations: dense vector with a liveness filter — each live
  // relation exactly once, in id order. Iterating access_ is not an option:
  // a relation is listed in the neighborhoods of both endpoints and would be
  // visited twice.
  template <LiveRelationCallback Fn>
  void ForEachLiveRelation(Fn&& fn) const {
    for (const Relation& relation : relations_) {
      if (HasRelation(relation.id()) && !fn(relation)) {
        return;
      }
    }
  }

  static Graph FromSnapshot(const GraphSnapshot& src);
  GraphSnapshot ToSnapshot() const;

  [[nodiscard]] bool SessionEqual(const Graph&) const;

 private:
  std::unordered_map<std::string, NodeType> ontology_node_;
  std::unordered_map<std::string, RelationType> ontology_relation_;

  // The type of every node and every relation is present in the ontology dictionaries.
  std::vector<Node> nodes_;
  std::vector<Relation> relations_;  // Relations reference only existing nodes: from and to of
                                     // every live relation are live nodes.

  // For any relation r and any node x: r is listed in `access_[x]` if and only if x is from or to
  // of r. `access_` holds entries exactly for live nodes: every live node has an entry (possibly
  // with an empty vector), no deleted node has one.
  std::map<NodeId, std::vector<RelationId>>
      access_;  // deterministic is necessary for serialization

  std::vector<std::optional<Position>> positions_;

  // Ids are never reused; `next_id_node_` / `next_id_relation_` are strictly greater than all
  // existing ids of their kind.
  NodeId next_id_node_ = 1;
  RelationId next_id_relation_ = 1;

  static constexpr std::uint64_t kMaxEntities = 1000000;
};

}  // namespace ravel

#endif  // RAVEL_CORE_INCLUDE_GRAPH_HPP_

#ifndef RAVEL_CORE_INCLUDE_GRAPH_HPP_
#define RAVEL_CORE_INCLUDE_GRAPH_HPP_

#include <cctype>
#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ravel {

using NodeId = std::uint64_t;
using RelationId = std::uint64_t;

inline bool isValidHexColor(const std::string& color) {
  if ((color.length() != 7) || (color[0] != '#')) {
    return false;
  }
  for (int i = 1; i < 7; ++i) {
    if (!std::isxdigit(static_cast<unsigned char>(color[i]))) {
      return false;
    }
  }
  return true;
}
inline bool isValidShape(const std::string& shape) {
  if ((shape == "rect") || shape == "oval") {
    return true;
  } else {
    return false;
  }
}

class NodeType {
 public:
  NodeType(std::string color, std::string shape, std::string frame_color)
      : color_(isValidHexColor(color)
                   ? color
                   : throw std::logic_error(
                         "Graph::NodeType tried to create Nodetype with non-valid color")),
        shape_(isValidShape(shape)
                   ? shape
                   : throw std::logic_error(
                         "Graph::NodeType tried to create Nodetype with non-valid shape")),
        frame_color_(
            isValidHexColor(frame_color)
                ? frame_color
                : throw std::logic_error(
                      "Graph::NodeType tried to create Nodetype with non-valid frame_color")) {
  }

  const std::string& color() const noexcept {
    return color_;
  }
  const std::string& shape() const noexcept {
    return shape_;
  }
  const std::string& frame_color() const noexcept {
    return frame_color_;
  }
  bool operator==(const NodeType& other) const {
    return (color_ == other.color_) && (shape_ == other.shape_) &&
           (frame_color_ == other.frame_color_);
  }

 private:
  std::string color_ = "#000000";
  std::string shape_ = "rect";
  std::string frame_color_ = "#000000";
};

class Node {  // DTO
 public:
  Node(NodeId id, std::string title, std::string type, std::string description = "",
       double size = 1, std::optional<std::pair<double, double>> hand_position = std::nullopt)
      : id_(id),
        type_(std::move(type)),
        title_(std::move(title)),
        description_(std::move(description)),
        size_(size > 0 ? size
                       : throw std::logic_error(
                             "Graph::Node tried to create Node with non-positive size")),
        hand_position_(hand_position) {
  }

  NodeId id() const noexcept {
    return id_;
  }
  const std::string& type() const noexcept {
    return type_;
  }

  const std::string& title() const noexcept {
    return title_;
  }
  const std::string& description() const noexcept {
    return description_;
  }
  double size() const noexcept {
    return size_;
  }
  std::optional<std::pair<double, double>> hand_position() const {
    return hand_position_;
  }

  void set_hand_position(std::pair<double, double> hand_position) noexcept {
    hand_position_ = hand_position;
  }

  // Exact == for doubles is intentional: size_/hand_position_ are stored
  // values compared for identity after serialization round-trips, not results
  // of floating-point computation. A bit-level mismatch here is a storage bug
  // and must surface, not be masked by an epsilon. Approximate comparison is
  // for computed values (layout geometry) only. See implementation.md,
  // note to rule 18.
  bool operator==(const Node& other) const {
    return (id_ == other.id_) && (type_ == other.type_) && (title_ == other.title_) &&
           (description_ == other.description_) && (size_ == other.size_) &&
           (hand_position_ == other.hand_position_);
  }

 private:
  // Identity
  NodeId id_ = 0;
  std::string type_ = "";

  // Local data
  std::string title_ = "";
  std::string description_ = "";
  double size_ = 1;
  std::optional<std::pair<double, double>> hand_position_ = std::nullopt;
};

inline bool isValidArrow(const std::string& arrow) {
  if (arrow == "solid" || arrow == "dashed" || arrow == "dotted") {
    return true;
  } else {
    return false;
  }
}

class RelationType {
 public:
  RelationType(std::string color, std::string arrow, bool symmetric, bool transitive)
      : color_(isValidHexColor(color)
                   ? color
                   : throw std::logic_error(
                         "Graph::RelationType tried to create Relationtype with non-valid color")),
        arrow_(isValidArrow(arrow)
                   ? arrow
                   : throw std::logic_error(
                         "Graph::RelationType tried to create Relationtype with non-valid arrow")),
        symmetric_(symmetric),
        transitive_(transitive) {
  }

  const std::string& color() const noexcept {
    return color_;
  }
  const std::string& arrow() const noexcept {
    return arrow_;
  }
  bool symmetric() const noexcept {
    return symmetric_;
  }
  bool transitive() const noexcept {
    return transitive_;
  }
  bool operator==(const RelationType& other) const {
    return (color_ == other.color_) && (arrow_ == other.arrow_) &&
           (symmetric_ == other.symmetric_) && (transitive_ == other.transitive_);
  }

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

  RelationId id() const noexcept {
    return id_;
  }
  NodeId from() const noexcept {
    return from_;
  }
  NodeId to() const noexcept {
    return to_;
  }
  const std::string& type() const noexcept {
    return type_;
  }

  const std::string& description() const noexcept {
    return description_;
  }

  bool operator==(const Relation& other) const {
    return (id_ == other.id_) && (from_ == other.from_) && (to_ == other.to_) &&
           (type_ == other.type_) && (description_ == other.description_);
  }

 private:
  RelationId id_ = 0;
  NodeId from_ = 0;
  NodeId to_ = 0;
  std::string type_ = "";

  std::string description_ = "";
};

struct GraphSnapshot {
  std::unordered_map<std::string, NodeType> ontology_node;
  std::unordered_map<std::string, RelationType> ontology_relation;

  std::vector<Node> nodes;
  std::vector<Relation> relations;

  NodeId next_id_node = 1;
  RelationId next_id_relation = 1;
};

class Graph {
 public:
  NodeId next_id_node() const noexcept {
    return next_id_node_;
  }
  NodeId next_id_relation() const noexcept {
    return next_id_relation_;
  }
  const std::unordered_map<std::string, NodeType>& ontology_node()
      const noexcept {  /// @return Const
    /// reference to the type dictionary. Lifetime is bound to *this; do not store the reference
    /// beyond the Graph's lifetime.
    return ontology_node_;
  }
  const std::unordered_map<std::string, RelationType>& ontology_relation()
      const noexcept {  /// @return
    /// Const reference to the type dictionary. Lifetime is bound to *this; do not store the
    /// reference beyond the Graph's lifetime.
    return ontology_relation_;
  }

  void AddNodeType(std::string name, std::string color, std::string shape,
                   std::string frame_color) {
    if (ontology_node_.find(name) != ontology_node_.end()) {
      throw std::logic_error("Graph::AddNodeType tried to add the same NodeType");
    }
    ontology_node_.emplace(name,
                           NodeType(std::move(color), std::move(shape), std::move(frame_color)));
  }
  void AddRelationType(std::string name, std::string color, std::string arrow, bool symmetric,
                       bool transitive) {
    if (ontology_relation_.find(name) != ontology_relation_.end()) {
      throw std::logic_error("Graph::AddRelationType tried to add the same RelationType");
    }
    ontology_relation_.emplace(
        name, RelationType(std::move(color), std::move(arrow), symmetric, transitive));
  }
  const NodeType& ResolveNode(const std::string& name) const {
    return ontology_node_.at(name);
  }
  const RelationType& ResolveRelation(const std::string& name) const {
    return ontology_relation_.at(name);
  }

  const Node& GetNode(NodeId id) const {
    if (HasNode(id)) {
      return nodes_[id - 1];
    } else {
      throw std::out_of_range("Graph::GetNode tried to access non-existent Node");
    }
  }
  NodeId AddNode(std::string title, std::string type, std::string description = "", double size = 1,
                 std::optional<std::pair<double, double>> hand_position = std::nullopt);
  bool HasNode(NodeId id) const {
    return access_.find(id) != access_.end();
  }
  RelationId AddRelation(NodeId from, NodeId to, std::string type, std::string description = "");
  bool HasRelation(RelationId id) const noexcept {
    return ((id >= 1) && (id < next_id_relation_) && (relations_[id - 1].id() == id));
  }
  const Relation& GetRelation(RelationId id) const {
    if (HasRelation(id)) {
      return relations_[id - 1];
    } else {
      throw std::out_of_range("Graph::GetRelation tried to access non-existent Relation");
    }
  }
  NodeId AddNodeWithRelations(
      std::string title, std::string type, std::vector<std::pair<NodeId, std::string>> froms,
      std::vector<std::pair<NodeId, std::string>> tos, std::string description = "",
      double size = 1, std::optional<std::pair<double, double>> hand_position = std::nullopt);
  std::optional<std::pair<double, double>> MoveNode(NodeId id, std::pair<double, double> pos);
  void DeleteNodes(std::vector<NodeId> ids);
  void DeleteRelations(std::vector<RelationId> ids);
  const std::vector<RelationId>& GetIncidentRelations(NodeId id) const {
    if (!HasNode(id)) {
      throw std::out_of_range("Graph::GetIncidentRelations tried to access non-existent Node");
    } else {
      return access_.at(id);
    }
  }
  std::set<NodeId> GetNeighbors(NodeId id) const;

  static Graph FromSnapshot(const GraphSnapshot& src);
  GraphSnapshot ToSnapshot() const;

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
  std::map<NodeId, std::vector<RelationId>> access_;

  // Ids are never reused; `next_id_node_` / `next_id_relation_` are strictly greater than all
  // existing ids of their kind.
  NodeId next_id_node_ = 1;
  RelationId next_id_relation_ = 1;
};

}  // namespace ravel

#endif  // RAVEL_CORE_INCLUDE_GRAPH_HPP_
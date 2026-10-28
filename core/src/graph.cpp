#include "graph.hpp"

#include <unordered_set>

namespace ravel {

NodeId Graph::AddNode(std::string title, std::string type, std::string description, double size) {
  // validate
  if (!ontology_node_.contains(type)) {
    throw std::invalid_argument("Graph::AddNode tried to create node with non-existent type");
  }
  if (size <= 0) {
    throw std::invalid_argument("Graph::AddNode tried to create node with non-positive size");
  }
  if (next_id_node_ > kMaxEntities) {
    throw std::invalid_argument("Graph::AddNode tried to create node with id > kMaxEntities");
  }

  // apply
  const NodeId id = next_id_node_++;
  nodes_.emplace_back(id, std::move(title), std::move(type), std::move(description), size);
  access_.try_emplace(id);
  positions_.push_back(std::nullopt);
  return id;
}

bool Graph::HasNode(NodeId id) const noexcept {
  const bool is_not_zombie = (id >= 1) && (id < next_id_node_) && (nodes_[id - 1].id() == id);
  if (!is_not_zombie) {
    return false;
  }
  return access_.contains(id);
}

Node Graph::ChangeNode(NodeId id, const std::string& title, const std::string& type,
                       const std::string& description, double size) {
  // validate
  if (!HasNode(id)) {
    throw std::invalid_argument("Graph::ChangeNode tried to change non-existent Node");
  }
  if (!ontology_node_.contains(type)) {
    throw std::invalid_argument("Graph::ChangeNode tried to change Node with non-existent type");
  }
  if (!std::isfinite(size) || size <= 0) {
    throw std::invalid_argument(
        "Graph::ChangeNode tried to change Node with non-positive/inf size");
  }

  // apply
  Node src = nodes_[id - 1];
  if (title == src.title() && type == src.type() && description == src.description() &&
      size == src.size()) {
    return src;
  }
  nodes_[id - 1] = Node(id, title, type, description, size);
  return src;
}

bool Graph::HasRelation(RelationId id) const noexcept {
  const bool is_not_zombie =
      (id >= 1) && (id < next_id_relation_) && (relations_[id - 1].id() == id);
  if (!is_not_zombie) {
    return false;
  }
  const NodeId from = relations_[id - 1].from();
  const auto it = access_.find(from);
  return it != access_.end() &&
         std::find(it->second.begin(), it->second.end(), id) != it->second.end();
}

RelationId Graph::AddRelation(NodeId from, NodeId to, std::string type, std::string description) {
  // validate
  if (!ontology_relation_.contains(type)) {
    throw std::invalid_argument(
        "Graph::AddRelation tried to create relation with non-existent type");
  }
  if (!HasNode(from)) {
    throw std::invalid_argument(
        "Graph::AddRelation tried to create relation with non-existent node from");
  }
  if (!HasNode(to)) {
    throw std::invalid_argument(
        "Graph::AddRelation tried to create relation with non-existent node to");
  }
  if (next_id_relation_ > kMaxEntities) {
    throw std::invalid_argument(
        "Graph::AddRelation tried to create relation with id > kMaxEntities");
  }

  // apply
  const RelationId id = next_id_relation_++;
  relations_.emplace_back(id, from, to, std::move(type), std::move(description));
  access_[from].push_back(id);
  access_[to].push_back(id);
  return id;
}

Relation Graph::ChangeRelation(RelationId id, const std::string& type,
                               const std::string& description) {
  // validate
  if (!HasRelation(id)) {
    throw std::invalid_argument("Graph::ChangeRelation tried to change non-existent Relation");
  }
  if (!ontology_relation_.contains(type)) {
    throw std::invalid_argument(
        "Graph::ChangeRelation tried to change Relation with non-existent type");
  }

  // apply
  Relation src = relations_[id - 1];
  if (type == src.type() && description == src.description()) {
    return src;
  }
  relations_[id - 1] = Relation(id, src.from(), src.to(), type, description);
  return src;
}

NodeId Graph::AddNodeWithRelations(std::string title, const std::string& type,
                                   const std::vector<std::pair<NodeId, std::string>>& froms,
                                   const std::vector<std::pair<NodeId, std::string>>& tos,
                                   std::string description, double size) {
  // validate
  if (!ontology_node_.contains(type)) {
    throw std::invalid_argument(
        "Graph::AddNodeWithRelations tried to create node with non-existent type");
  }
  if (size <= 0) {
    throw std::invalid_argument(
        "Graph::AddNodeWithRelations tried to create node with non-positive size");
  }
  for (const std::pair<NodeId, std::string>& from : froms) {
    if (!HasNode(from.first) || (!ontology_relation_.contains(from.second))) {
      throw std::invalid_argument("Graph::AddNodeWithRelations wrong from");
    }
  }
  for (const std::pair<NodeId, std::string>& to : tos) {
    if (!HasNode(to.first) || (!ontology_relation_.contains(to.second))) {
      throw std::invalid_argument("Graph::AddNodeWithRelations wrong to");
    }
  }
  if (next_id_node_ > kMaxEntities) {
    throw std::invalid_argument(
        "Graph::AddNodeWithRelations tried to create node with id > kMaxEntities");
  }
  if ((next_id_relation_ + froms.size() + tos.size()) > kMaxEntities) {
    throw std::invalid_argument(
        "Graph::AddNodeWithRelations tried to create relation with id > kMaxEntities");
  }

  // apply
  const NodeId cur = AddNode(std::move(title), type, std::move(description), size);
  for (const std::pair<NodeId, std::string>& from : froms) {
    AddRelation(from.first, cur, from.second);
  }
  for (const std::pair<NodeId, std::string>& to : tos) {
    AddRelation(cur, to.first, to.second);
  }
  return cur;
}

std::set<NodeId> Graph::GetNeighbors(NodeId id) const {
  // validate
  if (!HasNode(id)) {
    throw std::invalid_argument("Graph::GetNeighbors tried to access non-existent Node");
  }

  // apply
  std::set<NodeId> neighbors;
  for (const RelationId rid : access_.at(id)) {
    const Relation& r = relations_[rid - 1];
    if (r.from() != id) {
      neighbors.insert(r.from());
    }
    if (r.to() != id) {
      neighbors.insert(r.to());
    }
  }
  return neighbors;
}

std::optional<Position> Graph::MoveNode(NodeId id, Position pos) {
  if (!HasNode(id) || !std::isfinite(pos.x_) || !std::isfinite(pos.y_)) {
    throw std::invalid_argument(
        "Graph::MoveNode tried to access non-existent Node or move to nan/inf");
  }
  std::optional<Position> prev_pos = positions_[id - 1];
  positions_[id - 1] = pos;
  return prev_pos;
}

std::vector<RelationId> Graph::Delete(const std::vector<NodeId>& node_ids,
                                      const std::vector<RelationId>& rel_ids) {
  // validate
  std::set<NodeId> unique_n;
  for (const NodeId id : node_ids) {
    if (!HasNode(id)) {
      throw std::invalid_argument("Graph::Delete tried to delete non-existent Node");
    }
    unique_n.insert(id);
  }
  std::set<RelationId> unique_r;
  for (const RelationId id : rel_ids) {
    if (!HasRelation(id)) {
      throw std::invalid_argument("Graph::Delete tried to delete non-existent Relation");
    }
    unique_r.insert(id);
  }
  // apply
  for (const NodeId id : unique_n) {
    const std::vector<RelationId> incident(access_[id]);
    unique_r.insert(incident.begin(),
                    incident.end());  // by the invariant, only live relations are listed there
    access_.erase(id);
  }
  EraseRelations(unique_r);
  return {unique_r.begin(), unique_r.end()};
}

void Graph::EraseRelations(const std::set<RelationId>& unique_ids) {
  // The liveness of every relation id and the uniqueness of the argument are
  // validated by the caller (Delete). Endpoint liveness is NOT assumed: by
  // the time Delete calls this, the deleted nodes are already gone from
  // access_ — touching access_[dead] via operator[] would silently re-create
  // an empty entry, and find() on it would return end(). Hence the guards.
  for (const RelationId id : unique_ids) {
    const NodeId from = relations_[id - 1].from();
    const NodeId to = relations_[id - 1].to();
    if (HasNode(from)) {
      access_[from].erase(std::find(access_[from].begin(), access_[from].end(), id));
    }
    if (HasNode(to)) {
      access_[to].erase(std::find(access_[to].begin(), access_[to].end(), id));
    }
  }
}

void Graph::RestoreNodes(std::vector<NodeId> ids) {
  // validate (asserts vanish under NDEBUG — [[maybe_unused]] keeps
  // -Werror quiet about the loop variable in Release)
  const std::set<NodeId> unique(ids.begin(), ids.end());
  for ([[maybe_unused]] const NodeId id : unique) {
    assert(id < next_id_node_);
    assert(!HasNode(id));
    assert(nodes_[id - 1].id() == id);
  }

  // apply
  for (const NodeId id : unique) {
    access_.try_emplace(id, std::vector<RelationId>());
  }
}

void Graph::RestoreRelations(std::vector<RelationId> ids) {
  // validate (asserts vanish under NDEBUG — [[maybe_unused]] keeps
  // -Werror quiet about the loop variable in Release)
  const std::set<RelationId> unique(ids.begin(), ids.end());
  for ([[maybe_unused]] const RelationId id : unique) {
    assert(!HasRelation(id));  // bug undo
    assert(id < next_id_relation_);
    assert(relations_[id - 1].id() == id);
    assert(HasNode(relations_[id - 1].from()));
    assert(HasNode(relations_[id - 1].to()));
  }

  // apply
  for (const RelationId id : unique) {
    access_[relations_[id - 1].from()].push_back(id);
    access_[relations_[id - 1].to()].push_back(id);
  }
}

// Cognitive complexity 42 > 25: intentional. This is a chain of fail-fast
// snapshot-invariant checks; decomposition into validators is a separate
// track (roadmap: accumulated validation errors, std::expected). Suppressed
// here rather than disabling the check globally.
Graph Graph::FromSnapshot(  // NOLINT(readability-function-cognitive-complexity)
    const GraphSnapshot& src) {
  // validate
  if (src.next_id_node > kMaxEntities) {
    throw std::runtime_error("Graph::FromSnapshot next_id_node > limit = 1 000 000");
  }
  if (src.next_id_relation > kMaxEntities) {
    throw std::runtime_error("Graph::FromSnapshot next_id_relation > limit = 1 000 000");
  }

  if (src.next_id_node == 0 || src.next_id_relation == 0) {
    throw std::runtime_error(
        "Graph::FromSnapshot Graph with next_id_node = 0 or next_id_relation = 0");
  }
  if (src.next_id_node == 1 && src.nodes.empty() && src.next_id_relation == 1 &&
      src.relations.empty()) {
    // Ontology must survive the empty-entities shortcut: a file can carry
    // types only (no nodes/relations yet) — dropping it would turn a valid
    // saved session into a graph where AddNode rejects every known type.
    Graph g;
    g.ontology_node_ = src.ontology_node;
    g.ontology_relation_ = src.ontology_relation;
    return g;
  }
  if (src.next_id_node == 1 &&
      (!src.nodes.empty() || src.next_id_relation != 1 || !src.relations.empty())) {
    throw std::runtime_error("Graph::FromSnapshot Graph with next_id_node = 1 has nodes/relations");
  }
  if (src.next_id_relation == 1 && !src.relations.empty()) {
    throw std::runtime_error("Graph::FromSnapshot Graph with next_id_relation = 1 has relations");
  }
  if (src.nodes.size() != src.positions.size()) {
    throw std::runtime_error("Graph::FromSnapshot positions size != nodes size");
  }

  std::unordered_set<NodeId> alive_nodes;
  for (const Node& i : src.nodes) {
    // invariant 5
    if (!src.ontology_node.contains(i.type())) {
      throw std::runtime_error("Graph::FromSnapshot tried to add node with non-existent type");
    }
    // invariant 1
    if (i.id() == 0 || !alive_nodes.insert(i.id()).second) {
      throw std::runtime_error("Graph::FromSnapshot tried to add node with id = 0 or repeated id");
    }
    if (i.id() >= src.next_id_node) {
      throw std::runtime_error("Graph::FromSnapshot tried to add node with id >= next_id");
    }
    if (i.size() <= 0 || !std::isfinite(i.size())) {
      throw std::runtime_error(
          "Graph::FromSnapshot tried to add node with non-positive/nan/inf size");
    }
  }
  std::unordered_set<RelationId> alive_relations;
  for (const Relation& i : src.relations) {
    // invariant 5
    if (!src.ontology_relation.contains(i.type())) {
      throw std::runtime_error("Graph::FromSnapshot tried to add relation with non-existent type");
    }
    // invariant 4
    if (alive_nodes.count(i.from()) != 1) {
      throw std::runtime_error("Graph::FromSnapshot tried to add relation with non-existent from");
    }
    if (alive_nodes.count(i.to()) != 1) {
      throw std::runtime_error("Graph::FromSnapshot tried to add relation with non-existent to");
    }
    // invariant 1
    if (i.id() == 0 || !alive_relations.insert(i.id()).second) {
      throw std::runtime_error(
          "Graph::FromSnapshot tried to add relation with id = 0 or repeated id");
    }
    if (i.id() >= src.next_id_relation) {
      throw std::runtime_error("Graph::FromSnapshot tried to add relation with id >= next_id");
    }
  }

  for (const std::optional<Position>& pos : src.positions) {
    if ((pos != std::nullopt) && (!std::isfinite(pos->x_) || !std::isfinite(pos->y_))) {
      throw std::runtime_error("Graph::FromSnapshot tried to add node with nan/inf position");
    }
  }
  // apply
  Graph g;
  g.next_id_node_ = src.next_id_node;
  g.next_id_relation_ = src.next_id_relation;

  g.ontology_node_ = src.ontology_node;
  g.ontology_relation_ = src.ontology_relation;

  // placeholder ADR-002 11
  g.nodes_ = std::vector<Node>(src.next_id_node - 1, Node(0, "", "", ""));
  g.positions_ = std::vector<std::optional<Position>>(src.next_id_node - 1, std::nullopt);
  for (std::size_t i = 0; i < src.nodes.size(); ++i) {
    const Node& node = src.nodes[i];
    g.nodes_[node.id() - 1] = node;
    g.positions_[node.id() - 1] = src.positions[i];
    g.access_.try_emplace(node.id());
  }
  // placeholder ADR-002 11
  g.relations_ = std::vector<Relation>(src.next_id_relation - 1, Relation(0, 0, 0, ""));
  for (const Relation& relation : src.relations) {
    g.relations_[relation.id() - 1] = relation;
    g.access_[relation.from()].push_back(relation.id());
    g.access_[relation.to()].push_back(relation.id());
  }

  return g;
}

GraphSnapshot Graph::ToSnapshot() const {
  GraphSnapshot dest;
  dest.next_id_node = next_id_node_;
  dest.next_id_relation = next_id_relation_;
  dest.ontology_node = ontology_node_;
  dest.ontology_relation = ontology_relation_;

  ForEachLiveNode([&dest, this](const Node& node) {
    dest.nodes.push_back(node);
    dest.positions.push_back(positions_[node.id() - 1]);
    return true;
  });
  ForEachLiveRelation([&dest](const Relation& relation) {
    dest.relations.push_back(relation);
    return true;
  });

  return dest;
}

bool Graph::SessionEqual(const Graph& other) const {
  if (next_id_node_ != other.next_id_node_ || next_id_relation_ != other.next_id_relation_ ||
      nodes_ != other.nodes_ || relations_ != other.relations_ || positions_ != other.positions_ ||
      ontology_node_ != other.ontology_node_ || ontology_relation_ != other.ontology_relation_ ||
      access_.size() != other.access_.size()) {
    return false;
  }
  for (const auto& [id, incident] : access_) {
    const auto it = other.access_.find(id);
    if (it == other.access_.end()) {
      return false;
    }
    std::vector<RelationId> lhs(incident.begin(), incident.end());
    std::vector<RelationId> rhs(it->second.begin(), it->second.end());
    std::sort(lhs.begin(), lhs.end());
    std::sort(rhs.begin(), rhs.end());
    if (lhs != rhs) {
      return false;
    }
  }
  return true;
}

}  // namespace ravel

#include "graph.hpp"

#include <algorithm>
#include <unordered_set>

namespace ravel {

NodeId Graph::AddNode(std::string title, std::string type, std::string description, double size,
                      std::optional<std::pair<double, double>> hand_position) {
  // validate
  if (ontology_node_.find(type) == ontology_node_.end()) {
    throw std::invalid_argument("Graph::AddNode tried to create node with non-existent type");
  }
  if (size <= 0) {
    throw std::invalid_argument("Graph::AddNode tried to create node with non-positive size");
  }

  // apply
  NodeId id = next_id_node_++;
  nodes_.emplace_back(id, std::move(title), std::move(type), std::move(description), size,
                      hand_position);
  access_.try_emplace(id);
  return id;
}

bool Graph::HasNode(NodeId id) const {
  const bool is_not_zombie = (id >= 1) && (id < next_id_node_) && (nodes_[id - 1].id() == id);
  if (!is_not_zombie) {
    return false;
  }
  return access_.contains(id);
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
  if (ontology_relation_.find(type) == ontology_relation_.end()) {
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

  // apply
  RelationId id = next_id_relation_++;
  relations_.emplace_back(id, from, to, std::move(type), std::move(description));
  access_[from].push_back(id);
  access_[to].push_back(id);
  return id;
}

NodeId Graph::AddNodeWithRelations(std::string title, std::string type,
                                   std::vector<std::pair<NodeId, std::string>> froms,
                                   std::vector<std::pair<NodeId, std::string>> tos,
                                   std::string description, double size,
                                   std::optional<std::pair<double, double>> hand_position) {
  // validate
  if (ontology_node_.find(type) == ontology_node_.end()) {
    throw std::invalid_argument(
        "Graph::AddNodeWithRelations tried to create node with non-existent type");
  }
  if (size <= 0) {
    throw std::invalid_argument(
        "Graph::AddNodeWithRelations tried to create node with non-positive size");
  }
  for (const std::pair<NodeId, std::string>& from : froms) {
    if (!HasNode(from.first) ||
        (ontology_relation_.find(from.second) == ontology_relation_.end())) {
      throw std::invalid_argument("Graph::AddNodeWithRelations wrong from");
    }
  }
  for (const std::pair<NodeId, std::string>& to : tos) {
    if (!HasNode(to.first) || (ontology_relation_.find(to.second) == ontology_relation_.end())) {
      throw std::invalid_argument("Graph::AddNodeWithRelations wrong to");
    }
  }

  // apply
  NodeId cur = AddNode(title, type, description, size, hand_position);
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
  for (RelationId rid : access_.at(id)) {
    if (GetRelation(rid).from() != id) {
      neighbors.insert(GetRelation(rid).from());
    }
    if (GetRelation(rid).to() != id) {
      neighbors.insert(GetRelation(rid).to());
    }
  }
  return neighbors;
}

std::optional<std::pair<double, double>> Graph::MoveNode(NodeId id, std::pair<double, double> pos) {
  if (!HasNode(id)) {
    throw std::invalid_argument("Graph::MoveNode tried to access non-existent Node");
  } else {
    std::optional<std::pair<double, double>> prev_pos = GetNode(id).hand_position();
    nodes_[id - 1].set_hand_position(pos);
    return prev_pos;
  }
}

void Graph::DeleteNodes(std::vector<NodeId> ids) {
  // validate
  std::set<NodeId> unique;
  for (NodeId id : ids) {
    if (!HasNode(id)) {
      throw std::invalid_argument("Graph::DeleteNodes tried to delete non-existent Node");
    }
    unique.insert(id);
  }
  // apply
  for (NodeId id : unique) {
    DeleteRelations(access_[id]);  // by the invariant, only live relations are listed there
    access_.erase(id);
  }
}

void Graph::DeleteRelations(std::vector<RelationId> ids) {
  // validate
  std::set<RelationId> unique;
  for (RelationId id : ids) {
    if (!HasRelation(id)) {
      throw std::invalid_argument("Graph::DeleteRelations tried to delete non-existent Relation");
    }
    unique.insert(id);
  }
  // apply
  for (RelationId id : unique) {
    NodeId from = relations_[id - 1].from();
    NodeId to = relations_[id - 1].to();
    access_[from].erase(std::find(access_[from].begin(), access_[from].end(), id));
    access_[to].erase(std::find(access_[to].begin(), access_[to].end(), id));
  }
}

void Graph::RestoreNodes(std::vector<NodeId> ids) {
  // validate
  std::set<NodeId> unique(ids.begin(), ids.end());
  for (NodeId id : ids) {
    assert(id < next_id_node_);
    assert(!HasNode(id));
  }

  // apply
  for (NodeId id : ids) {
    access_.emplace(id, std::vector<RelationId>());
  }
}

void Graph::RestoreRelations(std::vector<RelationId> ids) {
  // validate
  const std::set<RelationId> unique(ids.begin(), ids.end());
  for (RelationId id : unique) {
    assert(!HasRelation(id));  // bug undo
    assert(HasNode(relations_[id - 1].from()));
    assert(HasNode(relations_[id - 1].to()));
  }

  // apply
  for (RelationId id : unique) {
    access_[relations_[id - 1].from()].push_back(id);
    access_[relations_[id - 1].to()].push_back(id);
  }
}

Graph Graph::FromSnapshot(const GraphSnapshot& src) {
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
    return Graph();
  }
  if (src.next_id_node == 1 &&
      (!src.nodes.empty() || src.next_id_relation != 1 || !src.relations.empty())) {
    throw std::runtime_error("Graph::FromSnapshot Graph with next_id_node = 1 has nodes/relations");
  }
  if (src.next_id_relation == 1 && !src.relations.empty()) {
    throw std::runtime_error("Graph::FromSnapshot Graph with next_id_relation = 1 has relations");
  }

  std::unordered_set<NodeId> alive_nodes;
  for (const Node& i : src.nodes) {
    // invariant 5
    if (src.ontology_node.find(i.type()) == src.ontology_node.end()) {
      throw std::runtime_error("Graph::FromSnapshot tried to add node with non-existent type");
    }
    // invariant 1
    if (i.id() == 0 || !alive_nodes.insert(i.id()).second) {
      throw std::runtime_error("Graph::FromSnapshot tried to add node with id = 0 or repeated id");
    }
    if (i.id() >= src.next_id_node) {
      throw std::runtime_error("Graph::FromSnapshot tried to add node with id >= next_id");
    }
    if (i.size() <= 0) {
      throw std::runtime_error("Graph::FromSnapshot tried to add node with non-positive size");
    }
  }
  std::unordered_set<RelationId> alive_relations;
  for (const Relation& i : src.relations) {
    // invariant 5
    if (src.ontology_relation.find(i.type()) == src.ontology_relation.end()) {
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
  // apply
  Graph g;
  g.next_id_node_ = src.next_id_node;
  g.next_id_relation_ = src.next_id_relation;

  g.ontology_node_ = src.ontology_node;
  g.ontology_relation_ = src.ontology_relation;

  // placeholder ADR-002 11
  g.nodes_ = std::vector<Node>(src.next_id_node - 1, Node(0, "", "", ""));
  for (const Node& node : src.nodes) {
    g.nodes_[node.id() - 1] = node;
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

  ForEachLiveNode([&dest](const Node& node) {
    dest.nodes.push_back(node);
    return true;
  });
  ForEachLiveRelation([&dest](const Relation& relation) {
    dest.relations.push_back(relation);
    return true;
  });

  return dest;
}

bool Graph::operator==(const Graph& other) const {
  if (next_id_node_ != other.next_id_node_ || next_id_relation_ != other.next_id_relation_ ||
      nodes_ != other.nodes_ || relations_ != other.relations_ ||
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

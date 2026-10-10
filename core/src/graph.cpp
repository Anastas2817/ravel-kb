#include "graph.hpp"

#include <algorithm>
#include <unordered_set>

namespace ravel {

NodeId Graph::AddNode(std::string title, std::string type, std::string description, double size,
                      std::optional<std::pair<double, double>> hand_position) {
  if (ontology_node_.find(type) == ontology_node_.end()) {
    throw std::out_of_range("Graph::AddNode tried to create node with non-existent type");
  }
  NodeId id = next_id_node_++;
  nodes_.emplace_back(id, std::move(title), std::move(type), std::move(description), size,
                      hand_position);
  access_.try_emplace(id);
  return id;
}

RelationId Graph::AddRelation(NodeId from, NodeId to, std::string type, std::string description) {
  if (ontology_relation_.find(type) == ontology_relation_.end()) {
    throw std::out_of_range("Graph::AddRelation tried to create relation with non-existent type");
  }
  if (!HasNode(from)) {
    throw std::out_of_range(
        "Graph::AddRelation tried to create relation with non-existent node from");
  }
  if (!HasNode(to)) {
    throw std::out_of_range(
        "Graph::AddRelation tried to create relation with non-existent node to");
  }
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
  for (const std::pair<NodeId, std::string>& from : froms) {
    if (!HasNode(from.first) ||
        (ontology_relation_.find(from.second) == ontology_relation_.end())) {
      throw std::out_of_range("Graph::AddNodeWithRelations");
    }
  }
  for (const std::pair<NodeId, std::string>& to : tos) {
    if (!HasNode(to.first) || (ontology_relation_.find(to.second) == ontology_relation_.end())) {
      throw std::out_of_range("Graph::AddNodeWithRelations");
    }
  }

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
  if (!HasNode(id)) {
    throw std::out_of_range("Graph::GetNeighbors tried to access non-existent Node");
  }

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
    throw std::out_of_range("Graph::MoveNode tried to access non-existent Node");
  } else {
    std::optional<std::pair<double, double>> prev_pos = GetNode(id).hand_position();
    nodes_[id - 1].set_hand_position(pos);
    return prev_pos;
  }
}

void Graph::DeleteNodes(std::vector<NodeId> ids) {
  for (NodeId id : ids) {
    if (!HasNode(id)) {
      throw std::out_of_range("Graph::DeleteNodes tried to delete non-existent Node");
    }
  }
  for (NodeId id : ids) {
    nodes_[id - 1] = Node(0, "", "");
    DeleteRelations(access_[id]);  // by the invariant, only live relations are listed there
    access_.erase(id);
  }
}

void Graph::DeleteRelations(std::vector<RelationId> ids) {
  for (RelationId id : ids) {
    if (!HasRelation(id)) {
      throw std::out_of_range("Graph::DeleteRelations tried to delete non-existent Relation");
    }
  }
  for (RelationId id : ids) {
    NodeId from = relations_[id - 1].from();
    NodeId to = relations_[id - 1].to();
    if (HasNode(from)) {
      access_[from].erase(std::find(access_[from].begin(), access_[from].end(), id));
    }
    if (HasNode(to)) {
      access_[to].erase(std::find(access_[to].begin(), access_[to].end(), id));
    }
    relations_[id - 1] = Relation(0, 0, 0, "");
  }
}

Graph Graph::FromSnapshot(const GraphSnapshot& src) {
  // validate
  const std::uint64_t kMaxEntities = 1000000;
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
    throw std::runtime_error("Graph::FromSnapshot Graph with next_id_node = 0 has nodes/relations");
  }
  if (src.next_id_relation == 1 && !src.relations.empty()) {
    throw std::runtime_error("Graph::FromSnapshot Graph with next_id_relation = 0 has relations");
  }
  // invariant 1
  if (!src.nodes.empty() && src.nodes.back().id() >= src.next_id_node) {
    throw std::runtime_error("Graph::FromSnapshot exists node_id >= next_id_node");
  }
  if (!src.relations.empty() && src.relations.back().id() >= src.next_id_relation) {
    throw std::runtime_error("Graph::FromSnapshot exists relation_id >= next_id_relation");
  }
  // invariant 3 and 4
  std::unordered_set<NodeId> alive_nodes;
  for (const Node& i : src.nodes) {
    if (src.ontology_node.find(i.type()) == src.ontology_node.end()) {
      throw std::runtime_error("Graph::FromSnapshot tried to add node with non-existent type");
    }
    if (i.id() == 0 || !alive_nodes.insert(i.id()).second) {
      throw std::runtime_error("Graph::FromSnapshot tried to add node with id = 0 or repeated id");
    }
  }
  std::unordered_set<RelationId> alive_relations;
  for (const Relation& i : src.relations) {
    if (src.ontology_relation.find(i.type()) == src.ontology_relation.end()) {
      throw std::runtime_error("Graph::FromSnapshot tried to add relation with non-existent type");
    }
    if (alive_nodes.count(i.from()) != 1) {
      throw std::runtime_error("Graph::FromSnapshot tried to add relation with non-existent from");
    }
    if (alive_nodes.count(i.to()) != 1) {
      throw std::runtime_error("Graph::FromSnapshot tried to add relation with non-existent to");
    }
    if (i.id() == 0 || !alive_relations.insert(i.id()).second) {
      throw std::runtime_error(
          "Graph::FromSnapshot tried to add relation with id = 0 or repeated id");
    }
  }
  // apply
  Graph g;
  g.next_id_node_ = src.next_id_node;
  g.next_id_relation_ = src.next_id_relation;

  g.ontology_node_ = src.ontology_node;
  g.ontology_relation_ = src.ontology_relation;

  g.nodes_ = std::vector<Node>(src.next_id_node - 1, Node(0, "", "", ""));
  for (const Node& node : src.nodes) {
    g.nodes_[node.id() - 1] = node;
    g.access_.try_emplace(node.id());
  }
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
  for (const auto& [key, value] : access_) {
    dest.nodes.push_back(nodes_[key - 1]);
  }
  for (RelationId id = 0; id < next_id_relation_; ++id) {
    if (HasRelation(id)) {
      dest.relations.push_back(relations_[id - 1]);
    }
  }
  return dest;
}

}  // namespace ravel
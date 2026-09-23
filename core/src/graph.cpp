#include <algorithm>

#include "graph.hpp"

NodeId Graph::AddNode(std::string title, std::string type, std::string description, double size, std::optional<std::pair<double, double>> hand_position) {
  if (ontology_node_.find(type) == ontology_node_.end()) throw std::out_of_range("Graph::AddNode try to create node with non-existent type");
  NodeId id = next_id_node_++;
  nodes_.emplace_back(id, std::move(title), std::move(type), std::move(description), size, hand_position);
  access_.try_emplace(id);
  return id;
}

RelationId Graph::AddRelation(NodeId from, NodeId to, std::string type, std::string description) {
  if (ontology_relation_.find(type) == ontology_relation_.end()) throw std::out_of_range("Graph::AddRelation try to create relation with non-existent type");
  if (HasNode(from) == false) throw std::out_of_range("Graph::AddRelation try to create relation with non-existent node from");
  if (HasNode(to) == false) throw std::out_of_range("Graph::AddRelation try to create relation with non-existent node to");
  RelationId id = next_id_relation_++;
  relations_.emplace_back(id, from, to, std::move(type), std::move(description));
  access_[from].push_back(id);
  access_[to].push_back(id);
  return id;
}

void Graph::DeleteNodes(std::vector<NodeId> ids) {
  for (NodeId id : ids) {
    if (HasNode(id)) {
      nodes_[id - 1] = Node(0, "", "");
      DeleteRelations(access_[id]); // из-за инварианта там только существующие рёбра
      access_.erase(id);
    }
  }
}

void Graph::DeleteRelations(std::vector<RelationId> ids) {
  for (RelationId id : ids) {
    NodeId from = relations_[id - 1].From();
    NodeId to = relations_[id - 1].To();
    if (HasNode(from)) access_[from].erase(std::find(access_[from].begin(), access_[from].end(), id)); // возможно лучше set для списка рёбер
    if (HasNode(to)) access_[to].erase(std::find(access_[to].begin(), access_[to].end(), id));
    relations_[id - 1] = Relation(0, 0, 0, "");
  }
}

void Graph::MoveNode(NodeId id, const std::pair<double, double> pos) {

}
#pragma once // not Google C++ style guide (((

#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using NodeId = std::uint64_t;
using RelationId = std::uint64_t;

class NodeType {
 public:
  NodeType(std::string color, std::string shape, std::string frame_color)
    : color_(color)
    , shape_(shape)
    , frame_color_(frame_color)
    { 
    }
 private:
  std::string color_ = "#000000";
  std::string shape_ = "rect";
  std::string frame_color_ = "#000000";
};

class Node { //DTO
 public:
  Node(NodeId id, std::string title, std::string type, std::string description = "", double size = 1, std::optional<std::pair<double, double>> hand_position = std::nullopt)
     : id_(id)
     , title_(std::move(title))
     , type_(std::move(type))
     , description_(std::move(description))
     , size_(size)
     , hand_position_(hand_position)
  {
  }

  NodeId Id() const noexcept { return id_; }
  const std::string& Type() const noexcept { return type_; }

  const std::string& Title() const noexcept { return title_; }
  const std::string& Description() const noexcept { return description_; }
  double Size() const noexcept { return size_; }
  std::optional<std::pair<double, double>> HandPosition() const { return hand_position_; }

  void SetHandPosition(std::pair<double, double> hand_position) noexcept { hand_position_ = hand_position; }

 private:
  // Идентичность
  NodeId id_ = 1;
  std::string type_ = "model";

  // Локальные данные
  std::string title_ = "";
  std::string description_ = "";
  double size_ = 1;
  std::optional<std::pair<double, double>> hand_position_ = std::nullopt;
};

class RelationType { 
 public:
  RelationType(std::string color, std::string arrow, bool symmetric, bool transitive)
    : color_(color)
    , arrow_(arrow)
    , symmetric_(symmetric)
    , transitive_(transitive)
    { 
    }
 private:
  std::string color_ = "#000000";
  std::string arrow_ = "solid"; // solid, dashed, dotted
  bool symmetric_ = false;
  bool transitive_ = false;
};

class Relation { //DTO
 public:
  Relation(RelationId id, NodeId from, NodeId to, std::string type, std::string description = "")
    : id_(id)
    , from_(from)
    , to_(to)
    , type_(std::move(type))
    , description_(std::move(description))
  {
  }

  RelationId Id() const noexcept { return id_; }
  NodeId From() const noexcept { return from_; }
  NodeId To() const noexcept { return to_; }
  const std::string& Type() const noexcept { return type_; }

  const std::string& Description() const noexcept { return description_; }

 private:
  RelationId id_ = 1;
  NodeId from_ = 1;
  NodeId to_ = 1;
  std::string type_ = "is_a";

  std::string description_ = "";
};

class Graph {
 public:
  NodeId NextIdNode() const noexcept { return next_id_node_; } // question for Storage
  NodeId NextIdRelation() const noexcept { return next_id_relation_; } // question for Storage

  void AddNodeType(std::string name, std::string color, std::string shape, std::string frame_color) {
    if (ontology_node_.find(name) != ontology_node_.end()) { throw std::logic_error("Graph::AddNodeType tried to add the same NodeType"); }
    ontology_node_.emplace(name, NodeType(std::move(color), std::move(shape), std::move(frame_color)));
  }
  void AddRelationType(std::string name, std::string color, std::string arrow, bool symmetric, bool transitive) {
    if (ontology_relation_.find(name) != ontology_relation_.end()) { throw std::logic_error("Graph::AddRelationType tried to add the same RelationType"); }
    ontology_relation_.emplace(name, RelationType(std::move(color), std::move(arrow), symmetric, transitive));
  }
  const NodeType& ResolveNode(const std::string& name) const { return ontology_node_.at(name); }
  const RelationType& ResolveRelation(const std::string& name) const { return ontology_relation_.at(name); }

  const Node& GetNode(NodeId id) const { if (HasNode(id)) { return nodes_[id - 1]; } else { throw std::out_of_range("Graph::GetNode tried to access non-existent Node"); } }
  NodeId AddNode(std::string title, std::string type, std::string description = "", double size = 1, std::optional<std::pair<double, double>> hand_position = std::nullopt);
  bool HasNode(NodeId id) const { return access_.find(id) != access_.end(); }
  RelationId AddRelation(NodeId from, NodeId to, std::string type, std::string description = "");
  bool HasRelation(RelationId id) const noexcept { return ((id >= 1) && (id < next_id_relation_) && (relations_[id - 1].Id() == id)); }
  const Relation& GetRelation(RelationId id) const { if (HasRelation(id)) { return relations_[id - 1]; } else { throw std::out_of_range("Graph::GetRelation tried to access non-existent Relation"); } }
  NodeId AddNodeWithRelations(std::string title, std::string type, std::vector<std::pair<NodeId, std::string>> froms, std::vector<std::pair<NodeId, std::string>> tos, std::string description = "", double size = 1, std::optional<std::pair<double, double>> hand_position = std::nullopt);
  std::optional<std::pair<double, double>> MoveNode(NodeId id, std::pair<double, double> pos);
  void DeleteNodes(std::vector<NodeId> ids);
  void DeleteRelations(std::vector<RelationId> ids);
  const std::vector<RelationId>& GetIncidentRelations(NodeId id) const { if (!HasNode(id)) { throw std::out_of_range("Graph::GetIncidentRelations tried to access non-existent Node"); } else { return access_.at(id); } }
  std::set<NodeId> GetNeighbors(NodeId id) const;

private:
  std::unordered_map<std::string, NodeType> ontology_node_;
  std::unordered_map<std::string, RelationType> ontology_relation_;

  // Тип каждого узла и каждого ребра содержится в соответствующем словаре онтологии.
  std::vector<Node> nodes_; 
  std::vector<Relation> relations_; // Рёбра ссылаются только на существующие узлы: from и to каждого живого ребра — живые узлы.

  // Для любого ребра r и любого узла x: r лежит в `access_[x]` тогда и только тогда, когда x — это from или to ребра r.
  // `access_` содержит записи ровно для живых узлов: каждый живый узел имеет запись (возможно, с пустым вектором), ни один удалённый — нет.
  std::map<NodeId, std::vector<RelationId>> access_;

  // Id не переиспользуются; `next_id_node_` / `next_id_relation_` строго больше всех существующих id своего вида.
  NodeId next_id_node_ = 1;
  RelationId next_id_relation_ = 1;
};
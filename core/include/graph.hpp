#pragma once // не Google C++ style guide (((

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using NodeId = std::uint64_t;
using RelationId = std::uint64_t;

class NodeType {
 private:
  std::string name_ = "";
  std::string color_ = "#000000";
  std::string shape_ = "rect";
  std::string frame_color_ = "#000000";
};

class Node {
 public:
  Node(NodeId id, std::string title, std::string type)
    : id_(id)
    , title_(title)
    , type_(type)
  {
  }

 private:
  NodeId id_ = 1;
  std::string title_ = "";
  std::string description_ = "";
  double size_ = 1;
  std::optional<std::pair<double, double>> hand_position_;
  std::string type_;
};

class RelationType {
 private:
  std::string name_ = "";
  std::string color_ = "#000000";
  std::string arrow_ = "solid"; // solid, dashed, dotted
  bool symmetric_ = false;
  bool transitive_ = false;
};

class Relation {
 public:
  Relation(RelationId id, NodeId from, NodeId to, std::string type)
    : id_(id)
    , from_(from)
    , to_(to)
    , type_(type)
  {
  }
 private:
  RelationId id_ = 1;
  std::string description_ = "";
  NodeId from_;
  NodeId to_;
  std::string type_;
};

class Graph {
 public:
  Graph() = default;
  Graph(const Graph&) = default;
  Graph(Graph&&) = delete;
  Graph& operator=(const Graph&) = delete;
  Graph& operator=(Graph&&) = delete;
  ~Graph() = default;

  NodeId get_next_id_node() const noexcept { return next_id_node_; }; // Вопрос к этапу Storage

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
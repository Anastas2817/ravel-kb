#pragma once // not Google C++ style guide (((

#include <cstdint>
#include <initializer_list>
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

class Node { //DTO
 public:
   Node(NodeId id, std::string title, std::string type, std::string description = "", double size = 1)
     : id_(id)
     , title_(std::move(title))
     , type_(std::move(type))
     , description_(std::move(description))
     , size_(size)
  {
  }

   NodeId get_id() const noexcept { return id_; }
   std::string get_title() const noexcept { return title_; }
   std::string get_type() const noexcept { return type_; }

   std::string get_description() const noexcept { return description_; }
   double get_size() const noexcept { return size_; }
   std::optional<std::pair<double, double>> get_hand_position() const noexcept { return hand_position_; }

 private:
  NodeId id_ = 1;
  std::string title_ = "";
  std::string type_ = "model";

  std::string description_ = "";
  double size_ = 1;
  std::optional<std::pair<double, double>> hand_position_;
};

class RelationType { 
 private:
  std::string name_ = "";
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

  RelationId get_id() const noexcept { return id_; }
  NodeId get_from() const noexcept { return from_; }
  NodeId get_to() const noexcept { return to_; }
  std::string get_type() const noexcept { return type_; }

  std::string get_description() const noexcept { return description_; }

 private:
  RelationId id_ = 1;
  NodeId from_ = 1;
  NodeId to_ = 1;
  std::string type_ = "is_a";

  std::string description_ = "";
};

class Graph {
 public:
  NodeId get_next_id_node() const noexcept { return next_id_node_; } // question for Storage
  NodeId get_next_id_node() const noexcept { return next_id_relation_; } // question for Storage

  void add_node(Node temp) {}
  void add_relation(Relation temp) {}
  void move_node(const std::pair<double, double> pos) {}
  void delete_node(const NodeId id) {}
  void delete_nodes(const std::initializer_list<NodeId> ids) {}

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
#ifdef _MSC_VER
#pragma warning(disable : 26495 26439)
#endif

#include "storage.hpp"

#include <gtest/gtest.h>

#include <sstream>

using ravel::Graph;
using ravel::JsonStorage;
using ravel::NodeId;
using ravel::RelationId;
using ravel::GraphSnapshot;
using ravel::NodeType;
using ravel::RelationType;
using ravel::Node;
using ravel::Relation;

TEST(StorageLoadSave, RoundTrip) {
  GraphSnapshot src;
  src.ontology_node.emplace("model", NodeType("#002030", "rect", "#a0500b"));
  src.ontology_relation.emplace("reduce_to", RelationType("#003d00", "dashed", false, true));

  src.nodes.push_back(Node(1, "Уравнения с разделяющимися переменными", "model", "f(x)dx=g(y)dy", 2,
                           std::make_pair(2.0, 3.0)));
  src.nodes.push_back(Node(2, "Уравнения, сводящиеся к однородным", "model"));
  src.nodes.push_back(Node(3, "Уравнения Бернулли", "model"));
  src.nodes.push_back(Node(4, "Однородные уравнения", "model"));

  src.relations.push_back(Relation(1, 2, 4, "reduce_to"));
  src.relations.push_back(Relation(1, 3, 4, "reduce_to"));
  src.relations.push_back(Relation(1, 4, 1, "reduce_to"));

  std::stringstream saved;
  JsonStorage::Save(src, saved);
  GraphSnapshot dest = JsonStorage::Load(saved);
  
  EXPECT_EQ(src.next_id_node, dest.next_id_node);
  EXPECT_EQ(src.next_id_relation, dest.next_id_relation);

  EXPECT_EQ(src.ontology_node, dest.ontology_node);
  EXPECT_EQ(src.ontology_relation, dest.ontology_relation);

  EXPECT_EQ(src.nodes, dest.nodes);
  EXPECT_EQ(src.relations, dest.relations);
}

TEST(StorageLoadSave, ValidEmpty) {
  GraphSnapshot src;
  std::stringstream saved;
  JsonStorage::Save(src, saved);
  GraphSnapshot dest = JsonStorage::Load(saved);

  const NodeId kStandartNodeIdValue = 1; // implementation: R1/R2
  const NodeId kStandartRelationIdValue = 1; // implementation: R1/R2
  EXPECT_EQ(dest.next_id_node, kStandartNodeIdValue);
  EXPECT_EQ(dest.next_id_relation, kStandartRelationIdValue);
}


TEST(StorageLoadSave, SurvivalOfLoners) {
  GraphSnapshot src;
  src.ontology_node.emplace("model", NodeType("#002030", "rect", "#a0500b"));
  src.ontology_relation.emplace("reduce_to", RelationType("#003d00", "dashed", false, true));
  src.nodes.push_back(Node(1, "Уравнения с разделяющимися переменными", "model", "f(x)dx=g(y)dy", 2,
                           std::make_pair(2.0, 3.0)));
  src.nodes.push_back(Node(2, "Уравнения, сводящиеся к однородным", "model"));
  src.nodes.push_back(Node(3, "Уравнения Бернулли", "model"));
  src.nodes.push_back(Node(4, "Однородные уравнения", "model"));

  src.relations.push_back(Relation(1, 2, 4, "reduce_to"));
  src.relations.push_back(Relation(1, 3, 4, "reduce_to"));
  src.relations.push_back(Relation(1, 4, 1, "reduce_to"));

  src.nodes.push_back(Node(5, "Уравнение в полных дифференциалах", "model"));

  std::stringstream saved;
  JsonStorage::Save(src, saved);
  GraphSnapshot dest = JsonStorage::Load(saved);
  
  EXPECT_EQ(src.next_id_node, dest.next_id_node);
  EXPECT_EQ(src.next_id_relation, dest.next_id_relation);

  EXPECT_EQ(src.ontology_node, dest.ontology_node);
  EXPECT_EQ(src.ontology_relation, dest.ontology_relation);

  EXPECT_EQ(src.nodes, dest.nodes);
  EXPECT_EQ(src.relations, dest.relations);
}


TEST(StorageLoad, WrongFormatVersion) {
  // Тест 1: format_version = 3 (не поддерживается)
  {
    std::stringstream ss{R"({
            "format_version": 3,
            "next_id_node": 3,
            "next_id_relation": 4,
            "node_types": [],
            "relation_types": [],
            "nodes": [],
            "relations": [],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }

  // Тест 2: нет format_version
  {
    std::stringstream ss{R"({
            "next_id_node": 3,
            "next_id_relation": 4,
            "node_types": [],
            "relation_types": [],
            "nodes": [],
            "relations": [],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }

  // Тест 3: format_version — строка вместо числа
  {
    std::stringstream ss{R"({
            "format_version": "2",
            "next_id_node": 3,
            "next_id_relation": 4,
            "node_types": [],
            "relation_types": [],
            "nodes": [],
            "relations": [],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }
}


TEST(StorageLoad, AbsenseOfAnObject) {
  // Тест 1: нет next_id_node
  {
    std::stringstream ss{R"({
            "format_version": 3,
            "next_id_relation": 4,
            "node_types": [],
            "relation_types": [],
            "nodes": [],
            "relations": [],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }

  // Тест 2: нет next_id_relation
  {
    std::stringstream ss{R"({
            "format_version": 3,
            "next_id_node": 3,
            "node_types": [],
            "relation_types": [],
            "nodes": [],
            "relations": [],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }

  // Тест 3: нет node_types
  {
    std::stringstream ss{R"({
            "format_version": "2",
            "next_id_node": 3,
            "next_id_relation": 4,
            "relation_types" : [],
            "nodes": [],
            "relations": [],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }

  // Тест 4: нет relation_types
  {
    std::stringstream ss{R"({
            "format_version": "2",
            "next_id_node": 3,
            "next_id_relation": 4,
            "node_types" : [],
            "nodes" : [],
            "relations": [],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }

  // Тест 5: нет nodes
  {
    std::stringstream ss{R"({
            "format_version": "2",
            "next_id_node": 3,
            "next_id_relation": 4,
            "node_types" : [],
            "relation_types" : [],
            "relations" : [],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }

  // Тест 6: нет relations
  {
    std::stringstream ss{R"({
            "format_version": "2",
            "next_id_node": 3,
            "next_id_relation": 4,
            "node_types" : [],
            "relation_types" : [],
            "nodes" : [],
            "allowed_cycles" : []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }
}


TEST(StorageLoad, WrongTypeObject) {
  // Тест 1: str next_id_node
  {
    std::stringstream ss{R"({
            "format_version": 3,
            "next_id_node" : "4"
            "next_id_relation": 4,
            "node_types": [],
            "relation_types": [],
            "nodes": [],
            "relations": [],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }

  // Тест 2: array next_id_relation
  {
    std::stringstream ss{R"({
            "format_version": 3,
            "next_id_node": 3,
            "next_id_relation" : [],
            "node_types": [],
            "relation_types": [],
            "nodes": [],
            "relations": [],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }

  // Тест 3: int node_types
  {
    std::stringstream ss{R"({
            "format_version": "2",
            "next_id_node": 3,
            "next_id_relation": 4,
            "node_types" : 7,
            "relation_types" : [],
            "nodes": [],
            "relations": [],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }

  // Тест 4: str relation_types
  {
    std::stringstream ss{R"({
            "format_version": "2",
            "next_id_node": 3,
            "next_id_relation": 4,
            "node_types" : [],
            "relation_types" : "wtf",
            "nodes" : [],
            "relations": [],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }

  // Тест 5: str nodes
  {
    std::stringstream ss{R"({
            "format_version": "2",
            "next_id_node": 3,
            "next_id_relation": 4,
            "node_types" : [],
            "relation_types" : [],
            "nodes" : "gg",
            "relations" : [],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }

  // Тест 6: int relations
  {
    std::stringstream ss{R"({
            "format_version": "2",
            "next_id_node": 3,
            "next_id_relation": 4,
            "node_types" : [],
            "relation_types" : [],
            "nodes" : [],
            "relations" : 97097,
            "allowed_cycles" : []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }

  // Тест 7: int allowed_cycles
  {
    std::stringstream ss{R"({
            "format_version": "2",
            "next_id_node": 3,
            "next_id_relation": 4,
            "node_types" : [],
            "relation_types" : [],
            "nodes" : [],
            "relations" : [],
            "allowed_cycles" : 68626
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }
}


TEST(StorageLoad, StrangeNodeType) {
  // Тест 1.1: имя отсутствует
  {
    std::stringstream ss{R"({
            "format_version": "2",
            "next_id_node": 3,
            "next_id_relation": 4,
            "node_types" : [
              {"color": "#4a90d9", "shape": "rect",
              "frame_color": "#2c5f94"}
            ],
            "relation_types" : [],
            "nodes": [],
            "relations": [],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }

  // Тест 2.1: color отсутствует
  {
    std::stringstream ss{R"({
            "format_version": "2",
            "next_id_node": 3,
            "next_id_relation": 4,
            "node_types" : [
              {"name": "#4a90d9", "shape": "rect",
              "frame_color": "#2c5f94"}
            ],
            "relation_types" : [],
            "nodes": [],
            "relations": [],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }

  // Тест 3.1: shape отсутствует
  {
    std::stringstream ss{R"({
            "format_version": "2",
            "next_id_node": 3,
            "next_id_relation": 4,
            "node_types" : [
              {"name": "#4a90d9", "color": "rect",
              "frame_color": "#2c5f94"}
            ],
            "relation_types" : [],
            "nodes": [],
            "relations": [],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }

  // Тест 4.1: frame_color отсутствует
  {
    std::stringstream ss{R"({
            "format_version": "2",
            "next_id_node": 3,
            "next_id_relation": 4,
            "node_types" : [
              {"name": "#4a90d9", "color": "rect",
              "shape": "#2c5f94"}
            ],
            "relation_types" : [],
            "nodes": [],
            "relations": [],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }

  // Тест 1.2: имя int
  {
    std::stringstream ss{R"({
            "format_version": "2",
            "next_id_node": 3,
            "next_id_relation": 4,
            "node_types" : [
              {"name": 67, "color": "#4a90d9", "shape": "rect",
              "frame_color": "#2c5f94"}
            ],
            "relation_types" : [],
            "nodes": [],
            "relations": [],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }

  // Тест 2.2: color array
  {
    std::stringstream ss{R"({
            "format_version": "2",
            "next_id_node": 3,
            "next_id_relation": 4,
            "node_types" : [
              {"name": "67", "color": [], "shape": "rect",
              "frame_color": "#2c5f94"}
            ],
            "relation_types" : [],
            "nodes": [],
            "relations": [],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }

  // Тест 3.2: shape array
  {
    std::stringstream ss{R"({
            "format_version": "2",
            "next_id_node": 3,
            "next_id_relation": 4,
            "node_types" : [
              {"name": "67", "color": "#000000", "shape": [],
              "frame_color": "#2c5f94"}
            ],
            "relation_types" : [],
            "nodes": [],
            "relations": [],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }

  // Тест 4.2: frame_color int
  {
    std::stringstream ss{R"({
            "format_version": "2",
            "next_id_node": 3,
            "next_id_relation": 4,
            "node_types" : [
              {"name": "67", "color": "#000000", "shape": "[]",
              "frame_color": 111119}
            ],
            "relation_types" : [],
            "nodes": [],
            "relations": [],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }
}


TEST(StorageLoad, StrangeRelationType) {
  // Тест 1.1: name отсутствует
  {
    std::stringstream ss{R"({
            "format_version": "2",
            "next_id_node": 3,
            "next_id_relation": 4,
            "node_types" : [],
            "relation_types" : [
              {"color": "#4a90d9", "arrow": "solid",
               "symmetric": false, "transitive": true}
            ],
            "nodes": [],
            "relations": [],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }
  // Тест 2.1: color отсутствует
  {
    std::stringstream ss{R"({
            "format_version": "2",
            "next_id_node": 3,
            "next_id_relation": 4,
            "node_types" : [],
            "relation_types" : [
              {"name": "is_a", "arrow": "solid",
               "symmetric": false, "transitive": true}
            ],
            "nodes": [],
            "relations": [],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }
  // Тест 3.1: arrow отсутствует
  {
    std::stringstream ss{R"({
            "format_version": "2",
            "next_id_node": 3,
            "next_id_relation": 4,
            "node_types" : [],
            "relation_types" : [
              {"name": "is_a", "color": "#4a90d9",
               "symmetric": false, "transitive": true}
            ],
            "nodes": [],
            "relations": [],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }
  // Тест 4.1: symmetric отсутствует
  {
    std::stringstream ss{R"({
            "format_version": "2",
            "next_id_node": 3,
            "next_id_relation": 4,
            "node_types" : [],
            "relation_types" : [
              {"name": "is_a", "color": "#4a90d9", "arrow": "solid",
               "transitive": true}
            ],
            "nodes": [],
            "relations": [],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }
  // Тест 5.1: transitive отсутствует
  {
    std::stringstream ss{R"({
            "format_version": "2",
            "next_id_node": 3,
            "next_id_relation": 4,
            "node_types" : [],
            "relation_types" : [
              {"name": "is_a", "color": "#4a90d9", "arrow": "solid",
               "symmetric": false}
            ],
            "nodes": [],
            "relations": [],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }
  // Тест 1.2: name array
  {
    std::stringstream ss{R"({
            "format_version": "2",
            "next_id_node": 3,
            "next_id_relation": 4,
            "node_types" : [],
            "relation_types" : [
              {"name": [], "color": "#4a90d9", "arrow": "solid",
               "symmetric": false, "transitive": true}
            ],
            "nodes": [],
            "relations": [],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }
  // Тест 2.2: color int
  {
    std::stringstream ss{R"({
            "format_version": "2",
            "next_id_node": 3,
            "next_id_relation": 4,
            "node_types" : [],
            "relation_types" : [
              {"name": "is_a", "color": 67, "arrow": "solid",
               "symmetric": false, "transitive": true}
            ],
            "nodes": [],
            "relations": [],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }
  // Тест 3.2: arrow array
  {
    std::stringstream ss{R"({
            "format_version": "2",
            "next_id_node": 3,
            "next_id_relation": 4,
            "node_types" : [],
            "relation_types" : [
              {"name": "is_a", "color": "#4a90d9", "arrow": [],
               "symmetric": false, "transitive": true}
            ],
            "nodes": [],
            "relations": [],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }
  // Тест 4.2: symmetric str
  {
    std::stringstream ss{R"({
            "format_version": "2",
            "next_id_node": 3,
            "next_id_relation": 4,
            "node_types" : [],
            "relation_types" : [
              {"name": "is_a", "color": "#4a90d9", "arrow": "solid",
               "symmetric": "false", "transitive": true}
            ],
            "nodes": [],
            "relations": [],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }
  // Тест 5.2: transitive int
  {
    std::stringstream ss{R"({
            "format_version": "2",
            "next_id_node": 3,
            "next_id_relation": 4,
            "node_types" : [],
            "relation_types" : [
              {"name": "is_a", "color": "#4a90d9", "arrow": "solid",
               "symmetric": false, "transitive": 1}
            ],
            "nodes": [],
            "relations": [],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }
}


TEST(StorageLoad, StrangeNode) {
  // Тест 1.1: id отсутствует
  {
    std::stringstream ss{R"({
            "format_version": "2",
            "next_id_node": 3,
            "next_id_relation": 4,
            "node_types" : [],
            "relation_types" : [],
            "nodes" : [
              {"title": "Степенная функция", "type": "function",
              "description": "f(x) = x^n", "hand_position": {"x": 120, "y": 340}}
            ],
            "relations": [],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }
  // Тест 2.1: title отсутствует
  {
    std::stringstream ss{R"({
            "format_version": "2",
            "next_id_node": 3,
            "next_id_relation": 4,
            "node_types" : [],
            "relation_types" : [],
            "nodes" : [
              {"id": 1, "type": "function",
              "description": "f(x) = x^n", "hand_position": {"x": 120, "y": 340}}
            ],
            "relations": [],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }
  // Тест 3.1: type отсутствует
  {
    std::stringstream ss{R"({
            "format_version": "2",
            "next_id_node": 3,
            "next_id_relation": 4,
            "node_types" : [],
            "relation_types" : [],
            "nodes" : [
              {"id": 1, "title": "Степенная функция",
              "description": "f(x) = x^n", "hand_position": {"x": 120, "y": 340}}
            ],
            "relations": [],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }
  // Тест 1.2: id str
  {
    std::stringstream ss{R"({
            "format_version": "2",
            "next_id_node": 3,
            "next_id_relation": 4,
            "node_types" : [],
            "relation_types" : [],
            "nodes" : [
              {"id": "max", "title": "Степенная функция", "type": "function",
              "description": "f(x) = x^n", "hand_position": {"x": 120, "y": 340}}
            ],
            "relations": [],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }
  // Тест 2.2: title int
  {
    std::stringstream ss{R"({
            "format_version": "2",
            "next_id_node": 3,
            "next_id_relation": 4,
            "node_types" : [],
            "relation_types" : [],
            "nodes" : [
              {"id": 1, "title": 56, "type": "function",
              "description": "f(x) = x^n", "hand_position": {"x": 120, "y": 340}}
            ],
            "relations": [],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }
  // Тест 3.2: type array
  {
    std::stringstream ss{R"({
            "format_version": "2",
            "next_id_node": 3,
            "next_id_relation": 4,
            "node_types" : [],
            "relation_types" : [],
            "nodes" : [
              {"id": 1, "title": "Степенная функция", "type": [],
              "description": "f(x) = x^n", "hand_position": {"x": 120, "y": 340}}
            ],
            "relations": [],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }
  // Тест 4.2: description array
  {
    std::stringstream ss{R"({
            "format_version": "2",
            "next_id_node": 3,
            "next_id_relation": 4,
            "node_types" : [],
            "relation_types" : [],
            "nodes" : [
              {"id": 1, "title": "Степенная функция", "type": "function",
              "description": [], "hand_position": {"x": 120, "y": 340}}
            ],
            "relations": [],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }
  // Тест 5.2: size str
  {
    std::stringstream ss{R"({
            "format_version": "2",
            "next_id_node": 3,
            "next_id_relation": 4,
            "node_types" : [],
            "relation_types" : [],
            "nodes" : [
              {"id": 1, "title": "Степенная функция", "type": "function",
              "description": "f(x) = x^n", "size": "big", "hand_position": {"x": 120, "y": 340}}
            ],
            "relations": [],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }
  // Тест 6.2: hand_position int
  {
    std::stringstream ss{R"({
            "format_version": "2",
            "next_id_node": 3,
            "next_id_relation": 4,
            "node_types" : [],
            "relation_types" : [],
            "nodes" : [
              {"id": 1, "title": "Степенная функция", "type": "function",
              "description": "f(x) = x^n", "hand_position": 120.340}
            ],
            "relations": [],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }
}


TEST(StorageLoad, StrangeRelation) {
  // Тест 1.1: id отсутствует
  {
    std::stringstream ss{R"({
            "format_version": "2",
            "next_id_node": 3,
            "next_id_relation": 4,
            "node_types" : [],
            "relation_types" : [],
            "nodes" : [],
            "relations" : [
              {"from": 1, "to": 3, "type": "is_defined_on",
               "description": "степенная функция с натуральным показателем задана на всей прямой"}
            ],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }
  // Тест 1.2: from отсутствует
  {
    std::stringstream ss{R"({
            "format_version": "2",
            "next_id_node": 3,
            "next_id_relation": 4,
            "node_types" : [],
            "relation_types" : [],
            "nodes" : [],
            "relations" : [
              {"id": 5, "to": 3, "type": "is_defined_on",
               "description": "степенная функция с натуральным показателем задана на всей прямой"}
            ],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }
  // Тест 3.1: to отсутствует
  {
    std::stringstream ss{R"({
            "format_version": "2",
            "next_id_node": 3,
            "next_id_relation": 4,
            "node_types" : [],
            "relation_types" : [],
            "nodes" : [],
            "relations" : [
              {"id": 5, "from": 1, "type": "is_defined_on",
               "description": "степенная функция с натуральным показателем задана на всей прямой"}
            ],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }
  // Тест 4.1: type отсутствует
  {
    std::stringstream ss{R"({
            "format_version": "2",
            "next_id_node": 3,
            "next_id_relation": 4,
            "node_types" : [],
            "relation_types" : [],
            "nodes" : [],
            "relations" : [
              {"id": 5, "from": 1, "to": 3,
               "description": "степенная функция с натуральным показателем задана на всей прямой"}
            ],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }
  // Тест 1.2: id array
  {
    std::stringstream ss{R"({
            "format_version": "2",
            "next_id_node": 3,
            "next_id_relation": 4,
            "node_types" : [],
            "relation_types" : [],
            "nodes" : [],
            "relations" : [
              {"id": [], "from": 1, "to": 3, "type": "is_defined_on",
               "description": "степенная функция с натуральным показателем задана на всей прямой"}
            ],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }
  // Тест 2.2: from str
  {
    std::stringstream ss{R"({
            "format_version": "2",
            "next_id_node": 3,
            "next_id_relation": 4,
            "node_types" : [],
            "relation_types" : [],
            "nodes" : [],
            "relations" : [
              {"id": 5, "from": "функция", "to": 3, "type": "is_defined_on",
               "description": "степенная функция с натуральным показателем задана на всей прямой"}
            ],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }
  // Тест 3.2: to str
  {
    std::stringstream ss{R"({
            "format_version": "2",
            "next_id_node": 3,
            "next_id_relation": 4,
            "node_types" : [],
            "relation_types" : [],
            "nodes" : [],
            "relations" : [
              {"id": 5, "from": 1, "to": "3", "type": "is_defined_on",
               "description": "степенная функция с натуральным показателем задана на всей прямой"}
            ],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }
  // Тест 4.2: type int
  {
    std::stringstream ss{R"({
            "format_version": "2",
            "next_id_node": 3,
            "next_id_relation": 4,
            "node_types" : [],
            "relation_types" : [],
            "nodes" : [],
            "relations" : [
              {"id": 5, "from": 1, "to": 3, "type": 567,
               "description": "степенная функция с натуральным показателем задана на всей прямой"}
            ],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }
  // Тест 5.2: description array
  {
    std::stringstream ss{R"({
            "format_version": "2",
            "next_id_node": 3,
            "next_id_relation": 4,
            "node_types" : [],
            "relation_types" : [],
            "nodes" : [],
            "relations" : [
              {"id": 5, "from": 1, "to": 3, "type": "is_defined_on",
               "description": []}
            ],
            "allowed_cycles": []
        })"};
    EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
  }
}


TEST(StorageLoad, FailStateStream) {
  std::stringstream ss;
  ss.setstate(std::ios::failbit);
  EXPECT_THROW(JsonStorage::Load(ss), std::runtime_error);
}


TEST(StorageSave, FailStateStream) {
  std::stringstream ss;
  ss.setstate(std::ios::failbit);
  GraphSnapshot g;
  EXPECT_THROW(JsonStorage::Save(g, ss), std::runtime_error);
}
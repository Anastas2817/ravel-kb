#pragma warning(disable : 26495 26439)

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
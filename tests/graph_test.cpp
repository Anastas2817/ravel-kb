#include <gtest/gtest.h>
#include "graph.hpp"

TEST(GraphAddNode, CreatesAccessEntryAndBumpsCounter) {
  Graph g;
  g.AddNodeType("model", "#4a90d9", "rect", "#000000");
  const NodeId id = g.AddNode("Вещественная прямая", "model");
  EXPECT_THROW(g.AddNode("Сложение", "function"), std::out_of_range); //инвариант 5: тип есть в словаре
  EXPECT_TRUE(g.HasNode(id));              // инвариант 3: запись в access_
  EXPECT_EQ(g.GetNode(id).Type(), "model");
  EXPECT_EQ(g.NextIdNode(), id + 1);  // инвариант 1: счётчик выдал id
}

TEST(GraphAddRelation, CreatesAccessEntryCounter) {
  Graph g;
  g.AddRelationType("has_special_case", "#3hj895", "solid", false, true); // здесь хотелось бы "1 является частным случаем 2", но поскольку общее сверху, то оно from
  // TODO : "#3hj895" не валидный формат для цвета
  g.AddNodeType("model", "#4a90d9", "rect", "#000000");
  NodeId from = g.AddNode("Признак Дирихле", "model");
  NodeId to = g.AddNode("Признак Лейбница", "model");
  const RelationId id = g.AddRelation(from, to, "has_special_case");
  EXPECT_THROW(g.AddRelation(from, 3, "has_special_case"), std::out_of_range); // инваринт 4: from и to существуют
  EXPECT_THROW(g.AddRelation(5, to, "has_special_case"), std::out_of_range); // инваринт 4: from и to существуют
  EXPECT_THROW(g.AddRelation(from, to, "is_a"), std::out_of_range); // инвариант  5: тип есть в словаре
  EXPECT_EQ(std::vector<RelationId>{id}, g.GetNeighborhood(from)); // инвариант 3: запись в access_
  EXPECT_EQ(std::vector<RelationId>{id}, g.GetNeighborhood(to)); // инвариант 3: запись в access_
  EXPECT_EQ(g.NextIdRelation(), id + 1); //инвариант 1: счётчик выдал id
}

TEST(GraphDeleteNodes, DeleteAccess) {
  Graph g;
  g.AddRelationType("has_special_case", "#3hj895", "solid", false, true); 
  g.AddNodeType("model", "#4a90d9", "rect", "#000000");
  NodeId from = g.AddNode("Признак Дирихле", "model");
  NodeId to1 = g.AddNode("Признак Лейбница", "model");
  NodeId to2 = g.AddNode("Признак Абеля", "model");
  const RelationId id1 = g.AddRelation(from, to1, "has_special_case");
  const RelationId id2 = g.AddRelation(from, to2, "has_special_case");
  g.DeleteNodes({from});
  EXPECT_FALSE(g.HasNode(from));
  EXPECT_FALSE(g.HasRelation(id1));
  EXPECT_FALSE(g.HasRelation(id2));
  EXPECT_THROW(g.GetRelation(id1), std::out_of_range);
  EXPECT_THROW(g.GetRelation(id2), std::out_of_range);
  EXPECT_TRUE(g.HasNode(to1) && g.HasNode(to2));
  EXPECT_EQ(std::vector<RelationId>{}, g.GetNeighborhood(to1)); // инвариант 3: запись в access_
  EXPECT_EQ(std::vector<RelationId>{}, g.GetNeighborhood(to2)); // инвариант 3: запись в access_
  EXPECT_EQ(g.NextIdNode(), to2 + 1); // инвариант 1: id не переиспользуются
}
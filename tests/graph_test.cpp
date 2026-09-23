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
  g.AddNodeType("model", "#4a90d9", "rect", "#000000");
  NodeId from = g.AddNode("Признак Дирихле", "model");
  NodeId to = g.AddNode("Признак Лейбница", "model");
  const RelationId id = g.AddRelation(from, to, "has_special_case");
  EXPECT_THROW(g.AddRelation(1, 3, "has_special_case"), std::out_of_range); // инваринт 4: from и to существуют
  EXPECT_THROW(g.AddRelation(5, 2, "has_special_case"), std::out_of_range); // инваринт 4: from и to существуют
  EXPECT_THROW(g.AddRelation(1, 2, "is_a"), std::out_of_range); // инвариант  5: тип есть в словаре
  EXPECT_TRUE(std::vector<RelationId>{1} == g.GetNeighborhood(1)); // инвариант 3: запись в access_
  EXPECT_TRUE(std::vector<RelationId>{1} == g.GetNeighborhood(2)); // инвариант 3: запись в access_
  EXPECT_EQ(g.NextIdRelation(), id + 1); //инвариант 1: счётчик выдал id
}
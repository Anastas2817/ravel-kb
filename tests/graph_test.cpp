#pragma warning(disable: 26495 26439)

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
  // TODO JsonStorage: "#3hj895" не валидный формат для цвета
  g.AddNodeType("model", "#4a90d9", "rect", "#000000");
  NodeId from = g.AddNode("Признак Дирихле", "model");
  NodeId to = g.AddNode("Признак Лейбница", "model");
  const RelationId id = g.AddRelation(from, to, "has_special_case");

  EXPECT_THROW(g.AddRelation(from, 3, "has_special_case"), std::out_of_range); // инваринт 4: from и to существуют
  EXPECT_THROW(g.AddRelation(5, to, "has_special_case"), std::out_of_range); // инваринт 4: from и to существуют
  EXPECT_THROW(g.AddRelation(from, to, "is_a"), std::out_of_range); // инвариант  5: тип есть в словаре

  EXPECT_EQ(std::vector<RelationId>{id}, g.GetIncidentRelations(from)); // инвариант 3: запись в access_
  EXPECT_EQ(std::vector<RelationId>{id}, g.GetIncidentRelations(to)); // инвариант 3: запись в access_
  EXPECT_EQ(g.NextIdRelation(), id + 1); //инвариант 1: счётчик выдал id
}

TEST(GraphHasRelation, NegativeRelation) {
  Graph g;
  g.AddNodeType("model", "#000000", "rect", "#000000");
  g.AddRelationType("isomorphic_to", "#000000", "solid", true, true);
  NodeId from = g.AddNode("ряд", "model");
  NodeId to = g.AddNode("последовательность", "model");
  RelationId cur = g.AddRelation(from, to, "isomorphic_to");
  EXPECT_TRUE(g.HasRelation(cur));
  EXPECT_FALSE(g.HasRelation(0));
  EXPECT_FALSE(g.HasRelation(999));
  EXPECT_FALSE(g.HasNode(0));
  EXPECT_FALSE(g.HasNode(999));
}

TEST(GraphDeleteNodes, DeleteAccess) {
  Graph g;
  g.AddRelationType("has_special_case", "#000000", "solid", false, true); 
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
  EXPECT_EQ(std::set<NodeId>{}, g.GetNeighbors(to1)); // инвариант 3: запись в access_
  EXPECT_EQ(std::set<NodeId>{}, g.GetNeighbors(to2)); // инвариант 3: запись в access_
  EXPECT_EQ(g.NextIdNode(), to2 + 1); // инвариант 1: id не переиспользуются
}

TEST(GraphMoveNode, SetNewHandPosition) {
  Graph g;
  g.AddNodeType("model", "#4a90d9", "rect", "#000000");
  NodeId id = g.AddNode("Датчик Холла", "model", "лаба по физике", 1, std::pair<double, double>{100, 100});
  std::optional<std::pair<double, double>> before = g.GetNode(id).HandPosition().value();
  EXPECT_EQ(before.value(), std::make_pair(100.0, 100.0));
  std::optional<std::pair<double, double>> prev = g.MoveNode(id, { -100, -100 });
  EXPECT_EQ(prev.value(), std::make_pair(100.0, 100.0));
  EXPECT_TRUE(g.HasNode(id));
  EXPECT_EQ(g.GetNode(id).HandPosition().value(), std::make_pair(-100.0, -100.0));

  NodeId temp = g.AddNode("Коаксильный кабель", "model");
  g.DeleteNodes({temp});
  EXPECT_THROW(g.MoveNode(temp, { -100, 100 }), std::out_of_range);

  NodeId primitive = g.AddNode("Магнитное поле", "model");
  std::optional<std::pair<double, double>> simple = g.MoveNode(primitive, { 30.0, 20.0 });
  EXPECT_EQ(simple, std::nullopt);
}

TEST(GraphGetNeighbors, ReturnsAdjacentNodes) {
  Graph g;
  g.AddNodeType("model", "#000000", "rect", "#000000");
  g.AddRelationType("is_a", "#000000", "solid", false, true);
  g.AddRelationType("has_part", "#000000", "solid", false, true);
  NodeId to = g.AddNode("фигура", "model");
  NodeId from1 = g.AddNode("круг", "model");
  NodeId from2 = g.AddNode("квадрат", "model");
  g.AddRelation(from1, to, "is_a");
  g.AddRelation(from2, to, "is_a");
  g.AddRelation(to, from1, "has_part");
  EXPECT_EQ(g.GetNeighbors(to), std::set<NodeId>({ from1, from2 }));
  EXPECT_EQ(g.GetNeighbors(from1), std::set<NodeId>({ to }));
  EXPECT_EQ(g.GetNeighbors(from2), std::set<NodeId>({ to }));
}

TEST(GraphAddNodeWithRelations, CreatesAccessEntry) {
  Graph g;
  g.AddNodeType("model", "#000000", "rect", "#000000");
  g.AddRelationType("reduce_to", "#000000", "solid", false, true);
  NodeId to1 = g.AddNode("Уравнения с разделяющимися переменными", "model");
  NodeId from1 = g.AddNode("Уравнения, сводящиеся к однородным", "model");
  NodeId from2 = g.AddNode("Уравнения Бернулли", "model");
  NodeId test = g.AddNodeWithRelations("Однородные уравнения", "model", { {from1, "reduce_to"}, {from2, "reduce_to"} }, { {to1, "reduce_to"} }); // 3 отношения
  EXPECT_TRUE(g.HasNode(test));
  EXPECT_EQ(g.GetNeighbors(test), std::set<NodeId>({ to1, from1, from2 }));
  EXPECT_EQ(g.GetNeighbors(to1), std::set<NodeId>{ test });
  EXPECT_EQ(g.GetNeighbors(from1), std::set<NodeId>({ test }));
  EXPECT_EQ(g.GetNeighbors(from2), std::set<NodeId>({ test }));
  EXPECT_EQ(g.NextIdNode(), test + 1);

  const size_t kAddedRelations = 3;

  EXPECT_EQ(g.NextIdRelation(), kAddedRelations + 1);

  EXPECT_THROW(g.AddNodeWithRelations("Однородные уравнения", "modl", { {from1, "reduce_to"}, {from2, "reduce_to"} }, { {to1, "reduce_to"} }), std::out_of_range);
  EXPECT_EQ(g.NextIdNode(), test + 1);
  EXPECT_EQ(g.NextIdRelation(), kAddedRelations + 1);

  EXPECT_THROW(g.AddNodeWithRelations("Однородные уравнения", "model", { {from1, "reduc_to"}, {from2, "reduce_to"} }, { {to1, "reduce_to"} }), std::out_of_range);
  EXPECT_EQ(g.NextIdNode(), test + 1);
  EXPECT_EQ(g.NextIdRelation(), kAddedRelations + 1);
}
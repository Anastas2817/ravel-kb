#ifdef _MSC_VER
#pragma warning(disable : 26495 26439)
#endif

#include "graph.hpp"

#include <gtest/gtest.h>

using ravel::Graph;
using ravel::GraphSnapshot;
using ravel::Node;
using ravel::NodeId;
using ravel::NodeType;
using ravel::Relation;
using ravel::RelationId;
using ravel::RelationType;

TEST(GraphAddNode, CreatesAccessEntryAndBumpsCounter) {
  Graph g;
  g.AddNodeType("model", "#4a90d9", "rect", "#000000");
  const NodeId id = g.AddNode("Вещественная прямая", "model");
  EXPECT_THROW(g.AddNode("Сложение", "function"),
               std::invalid_argument);  // инвариант 5: тип есть в словаре
  EXPECT_TRUE(g.HasNode(id));           // инвариант 3: запись в access_
  EXPECT_EQ(g.GetNode(id).type(), "model");
  EXPECT_EQ(g.next_id_node(), id + 1);  // инвариант 1: счётчик выдал id

  EXPECT_THROW(g.AddNode("Математика", "model", "царица наук", -467.0), std::invalid_argument);
}

TEST(GraphAddRelation, CreatesAccessEntryCounter) {
  Graph g;
  g.AddRelationType("has_special_case", "#3ba895", "solid", false, true);
  g.AddNodeType("model", "#4a90d9", "rect", "#000000");
  NodeId from = g.AddNode("Признак Дирихле", "model");
  NodeId to = g.AddNode("Признак Лейбница", "model");
  const RelationId id = g.AddRelation(from, to, "has_special_case");

  EXPECT_THROW(g.AddRelation(from, 3, "has_special_case"),
               std::invalid_argument);  // инваринт 4: from и to существуют
  EXPECT_THROW(g.AddRelation(5, to, "has_special_case"),
               std::invalid_argument);  // инваринт 4: from и to существуют
  EXPECT_THROW(g.AddRelation(from, to, "is_a"),
               std::invalid_argument);  // инвариант  5: тип есть в словаре

  EXPECT_EQ(std::vector<RelationId>{id},
            g.GetIncidentRelations(from));  // инвариант 3: запись в access_
  EXPECT_EQ(std::vector<RelationId>{id},
            g.GetIncidentRelations(to));    // инвариант 3: запись в access_
  EXPECT_EQ(g.next_id_relation(), id + 1);  // инвариант 1: счётчик выдал id
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
  EXPECT_THROW(g.GetRelation(id1), std::invalid_argument);
  EXPECT_THROW(g.GetRelation(id2), std::invalid_argument);
  EXPECT_TRUE(g.HasNode(to1) && g.HasNode(to2));
  EXPECT_EQ(std::set<NodeId>{}, g.GetNeighbors(to1));  // инвариант 3: запись в access_
  EXPECT_EQ(std::set<NodeId>{}, g.GetNeighbors(to2));  // инвариант 3: запись в access_
  EXPECT_EQ(g.next_id_node(), to2 + 1);                // инвариант 1: id не переиспользуются
}

TEST(GraphMoveNode, SetNewHandPosition) {
  Graph g;
  g.AddNodeType("model", "#4a90d9", "rect", "#000000");
  NodeId id =
      g.AddNode("Датчик Холла", "model", "лаба по физике", 1, std::pair<double, double>{100, 100});
  std::optional<std::pair<double, double>> before = g.GetNode(id).hand_position().value();
  EXPECT_EQ(before.value(), std::make_pair(100.0, 100.0));
  std::optional<std::pair<double, double>> prev = g.MoveNode(id, {-100, -100});
  EXPECT_EQ(prev.value(), std::make_pair(100.0, 100.0));
  EXPECT_TRUE(g.HasNode(id));
  EXPECT_EQ(g.GetNode(id).hand_position().value(), std::make_pair(-100.0, -100.0));

  NodeId temp = g.AddNode("Коаксильный кабель", "model");
  g.DeleteNodes({temp});
  EXPECT_THROW(g.MoveNode(temp, {-100, 100}), std::invalid_argument);

  NodeId primitive = g.AddNode("Магнитное поле", "model");
  std::optional<std::pair<double, double>> simple = g.MoveNode(primitive, {30.0, 20.0});
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
  EXPECT_EQ(g.GetNeighbors(to), std::set<NodeId>({from1, from2}));
  EXPECT_EQ(g.GetNeighbors(from1), std::set<NodeId>({to}));
  EXPECT_EQ(g.GetNeighbors(from2), std::set<NodeId>({to}));
}

TEST(GraphAddNodeWithRelations, CreatesAccessEntry) {
  Graph g;
  g.AddNodeType("model", "#000000", "rect", "#000000");
  g.AddRelationType("reduce_to", "#000000", "solid", false, true);
  NodeId to1 = g.AddNode("Уравнения с разделяющимися переменными", "model");
  NodeId from1 = g.AddNode("Уравнения, сводящиеся к однородным", "model");
  NodeId from2 = g.AddNode("Уравнения Бернулли", "model");
  NodeId test = g.AddNodeWithRelations("Однородные уравнения", "model",
                                       {{from1, "reduce_to"}, {from2, "reduce_to"}},
                                       {{to1, "reduce_to"}});  // 3 отношения
  EXPECT_TRUE(g.HasNode(test));
  EXPECT_EQ(g.GetNeighbors(test), std::set<NodeId>({to1, from1, from2}));
  EXPECT_EQ(g.GetNeighbors(to1), std::set<NodeId>{test});
  EXPECT_EQ(g.GetNeighbors(from1), std::set<NodeId>({test}));
  EXPECT_EQ(g.GetNeighbors(from2), std::set<NodeId>({test}));
  EXPECT_EQ(g.next_id_node(), test + 1);

  const size_t kAddedRelations = 3;

  EXPECT_EQ(g.next_id_relation(), kAddedRelations + 1);

  EXPECT_THROW(
      g.AddNodeWithRelations("Однородные уравнения", "modl",
                             {{from1, "reduce_to"}, {from2, "reduce_to"}}, {{to1, "reduce_to"}}),
      std::invalid_argument);
  EXPECT_EQ(g.next_id_node(), test + 1);
  EXPECT_EQ(g.next_id_relation(), kAddedRelations + 1);

  EXPECT_THROW(
      g.AddNodeWithRelations("Однородные уравнения", "model",
                             {{from1, "reduc_to"}, {from2, "reduce_to"}}, {{to1, "reduce_to"}}),
      std::invalid_argument);
  EXPECT_THROW(
      g.AddNodeWithRelations("Однородные уравнения", "modl",
                             {{from1, "reduce_to"}, {from2, "reduce_to"}}, {{to1, "reduce_to"}}),
      std::invalid_argument);
  EXPECT_THROW(g.AddNodeWithRelations("Однородные уравнения", "model",
                                      {{from1, "reduce_to"}, {from2, "reduce_to"}},
                                      {{to1, "reduce_to"}}, "", -100.0),
               std::invalid_argument);
  EXPECT_EQ(g.next_id_node(), test + 1);
  EXPECT_EQ(g.next_id_relation(), kAddedRelations + 1);
}

TEST(GraphFromSnapshot, ValidGraphWithCorrectInvariantsAndHoles) {
  GraphSnapshot src;
  src.next_id_node = 9;
  src.next_id_relation = 5;

  src.ontology_node.emplace("model", NodeType("#000000", "rect", "#000000"));
  src.ontology_relation.emplace("has_special_case", RelationType("#000000", "solid", false, true));

  const NodeId kAliveNode1 = 1;
  const NodeId kAliveNode2 = 2;
  const NodeId kAliveNode3 = 5;
  src.nodes.emplace_back(kAliveNode1, "Ряд Дирихле", "model");
  const double size = 4.0;
  const std::pair<double, double> hand_position{4.0, 8.0};
  src.nodes.emplace_back(kAliveNode2, "Гармонический ряд", "model", "1/n", size, hand_position);
  src.nodes.emplace_back(kAliveNode3, "Телескопический ряд", "model");
  const RelationId kAliveRelation = 3;
  src.relations.emplace_back(kAliveRelation, kAliveNode1, kAliveNode2, "has_special_case");

  Graph g = Graph::FromSnapshot(src);
  EXPECT_EQ(g.next_id_node(), src.next_id_node);
  EXPECT_EQ(g.next_id_relation(), src.next_id_relation);

  std::unordered_map<std::string, NodeType> right_node_types;
  right_node_types.emplace("model", NodeType("#000000", "rect", "#000000"));
  EXPECT_EQ(g.ontology_node(), right_node_types);

  std::unordered_map<std::string, RelationType> right_relation_types;
  right_relation_types.emplace("has_special_case", RelationType("#000000", "solid", false, true));
  EXPECT_EQ(g.ontology_relation(), right_relation_types);

  for (NodeId id = 0; id < src.next_id_node; ++id) {
    if (id == kAliveNode1 || id == kAliveNode2 || id == kAliveNode3) {
      EXPECT_TRUE(g.HasNode(id));
    } else {
      EXPECT_FALSE(g.HasNode(id));
    }
  }
  EXPECT_EQ(g.GetNode(kAliveNode1).id(), kAliveNode1);
  EXPECT_EQ(g.GetNode(kAliveNode1).title(), "Ряд Дирихле");
  EXPECT_EQ(g.GetNode(kAliveNode1).type(), "model");
  EXPECT_EQ(g.GetNode(kAliveNode2).id(), kAliveNode2);
  EXPECT_EQ(g.GetNode(kAliveNode2).title(), "Гармонический ряд");
  EXPECT_EQ(g.GetNode(kAliveNode2).type(), "model");
  EXPECT_EQ(g.GetNode(kAliveNode2).description(), "1/n");
  EXPECT_EQ(g.GetNode(kAliveNode2).size(), size);
  EXPECT_EQ(g.GetNode(kAliveNode2).hand_position(), hand_position);
  EXPECT_EQ(g.GetNode(kAliveNode3).id(), kAliveNode3);
  EXPECT_EQ(g.GetNode(kAliveNode3).title(), "Телескопический ряд");
  EXPECT_EQ(g.GetNode(kAliveNode3).type(), "model");

  for (RelationId id = 0; id < src.next_id_relation; ++id) {
    if (id == kAliveRelation) {
      EXPECT_TRUE(g.HasRelation(id));
    } else {
      EXPECT_FALSE(g.HasRelation(id));
    }
  }
  EXPECT_EQ(g.GetRelation(kAliveRelation).from(), kAliveNode1);
  EXPECT_EQ(g.GetRelation(kAliveRelation).to(), kAliveNode2);
  EXPECT_EQ(g.GetRelation(kAliveRelation).description(), "");
  EXPECT_EQ(g.GetRelation(kAliveRelation).type(), "has_special_case");

  EXPECT_EQ(g.GetIncidentRelations(kAliveNode1), std::vector<RelationId>({kAliveRelation}));
  EXPECT_EQ(g.GetIncidentRelations(kAliveNode2), std::vector<RelationId>({kAliveRelation}));
  EXPECT_EQ(g.GetIncidentRelations(kAliveNode3), std::vector<RelationId>());

  EXPECT_EQ(g.GetNeighbors(kAliveNode1), std::set<NodeId>({kAliveNode2}));
  EXPECT_EQ(g.GetNeighbors(kAliveNode2), std::set<NodeId>({kAliveNode1}));
  EXPECT_EQ(g.GetNeighbors(kAliveNode3), std::set<NodeId>());
}

TEST(GraphFromSnapshot, EmptySnapshot) {
  GraphSnapshot src;
  Graph g = Graph::FromSnapshot(src);
  EXPECT_EQ(g.next_id_node(), 1);
  EXPECT_EQ(g.next_id_relation(), 1);
  const std::unordered_map<std::string, NodeType> empty_node_type;
  EXPECT_EQ(g.ontology_node(), empty_node_type);
  const std::unordered_map<std::string, RelationType> empty_relation_type;
  EXPECT_EQ(g.ontology_relation(), empty_relation_type);
  EXPECT_FALSE(g.HasNode(1));
  EXPECT_FALSE(g.HasRelation(1));
}

TEST(GraphFromSnapshot, NextLessThanMaxId) {
  GraphSnapshot src;
  src.next_id_node = 9;
  src.next_id_relation = 5;

  src.ontology_node.emplace("model", NodeType("#000000", "rect", "#000000"));
  src.ontology_relation.emplace("has_special_case", RelationType("#000000", "solid", false, true));

  const NodeId kAliveNode1 = 1;
  const NodeId kAliveNode2 = 2;
  const NodeId kAliveNode3 = 11;
  src.nodes.emplace_back(kAliveNode1, "Ряд Дирихле", "model");
  const double size = 4.0;
  const std::pair<double, double> hand_position{4.0, 8.0};
  src.nodes.emplace_back(kAliveNode2, "Гармонический ряд", "model", "1/n", size, hand_position);
  src.nodes.emplace_back(kAliveNode3, "Телескопический ряд", "model");
  const RelationId kAliveRelation = 3;
  src.relations.emplace_back(kAliveRelation, kAliveNode1, kAliveNode2, "has_special_case");

  EXPECT_THROW(Graph::FromSnapshot(src), std::runtime_error);
}

TEST(GraphFromSnapshot, NonPositiveNodeSize) {
  // Инвариант size > 0 держит конструктор Node: битый узел не может
  // существовать в снимке, поэтому FromSnapshot его никогда не увидит.
  GraphSnapshot src;
  src.next_id_node = 9;
  src.next_id_relation = 5;

  src.ontology_node.emplace("model", NodeType("#000000", "rect", "#000000"));
  src.ontology_relation.emplace("has_special_case", RelationType("#000000", "solid", false, true));

  const NodeId kAliveNode1 = 1;
  const NodeId kAliveNode2 = 2;
  src.nodes.emplace_back(kAliveNode1, "Ряд Дирихле", "model");
  const double size = -4.0;
  EXPECT_THROW(src.nodes.emplace_back(kAliveNode2, "Гармонический ряд", "model", "1/n", size,
                                      std::pair<double, double>{4.0, 8.0}),
               std::invalid_argument);
  EXPECT_EQ(src.nodes.size(), 1);  // strong guarantee: снимок не тронут
}

TEST(GraphFromSnapshot, InvalidNodeType) {
  GraphSnapshot src;
  src.next_id_node = 9;
  src.next_id_relation = 5;

  src.ontology_node.emplace("model", NodeType("#000000", "rect", "#000000"));
  src.ontology_relation.emplace("has_special_case", RelationType("#000000", "solid", false, true));

  const NodeId kAliveNode1 = 1;
  const NodeId kAliveNode2 = 2;
  const NodeId kAliveNode3 = 5;
  src.nodes.emplace_back(kAliveNode1, "Ряд Дирихле", "mod");
  const double size = 4.0;
  const std::pair<double, double> hand_position{4.0, 8.0};
  src.nodes.emplace_back(kAliveNode2, "Гармонический ряд", "model", "1/n", size, hand_position);
  src.nodes.emplace_back(kAliveNode3, "Телескопический ряд", "model");
  const RelationId kAliveRelation = 3;
  src.relations.emplace_back(kAliveRelation, kAliveNode1, kAliveNode2, "has_special_case");

  EXPECT_THROW(Graph::FromSnapshot(src), std::runtime_error);
}

TEST(GraphFromSnapshot, InvalidRelationType) {
  GraphSnapshot src;
  src.next_id_node = 9;
  src.next_id_relation = 5;

  src.ontology_node.emplace("model", NodeType("#000000", "rect", "#000000"));
  src.ontology_relation.emplace("has_special_case", RelationType("#000000", "solid", false, true));

  const NodeId kAliveNode1 = 1;
  const NodeId kAliveNode2 = 2;
  const NodeId kAliveNode3 = 5;
  src.nodes.emplace_back(kAliveNode1, "Ряд Дирихле", "model");
  const double size = 4.0;
  const std::pair<double, double> hand_position{4.0, 8.0};
  src.nodes.emplace_back(kAliveNode2, "Гармонический ряд", "model", "1/n", size, hand_position);
  src.nodes.emplace_back(kAliveNode3, "Телескопический ряд", "model");
  const RelationId kAliveRelation = 3;
  src.relations.emplace_back(kAliveRelation, kAliveNode1, kAliveNode2, "has_ecial_case");

  EXPECT_THROW(Graph::FromSnapshot(src), std::runtime_error);
}

TEST(GraphFromSnapshot, RelationToNonExistentTo) {
  GraphSnapshot src;
  src.next_id_node = 9;
  src.next_id_relation = 5;

  src.ontology_node.emplace("model", NodeType("#000000", "rect", "#000000"));
  src.ontology_relation.emplace("has_special_case", RelationType("#000000", "solid", false, true));

  const NodeId kAliveNode1 = 1;
  const NodeId kAliveNode2 = 2;
  const NodeId kAliveNode3 = 5;
  src.nodes.emplace_back(kAliveNode1, "Ряд Дирихле", "mod");
  const double size = 4.0;
  const std::pair<double, double> hand_position{4.0, 8.0};
  src.nodes.emplace_back(kAliveNode2, "Гармонический ряд", "model", "1/n", size, hand_position);
  src.nodes.emplace_back(kAliveNode3, "Телескопический ряд", "model");
  const RelationId kAliveRelation = 3;
  const NodeId kDead = 56;
  src.relations.emplace_back(kAliveRelation, kAliveNode1, kDead, "has_special_case");

  EXPECT_THROW(Graph::FromSnapshot(src), std::runtime_error);
}

TEST(GraphFromSnapshot, RelationToNonExistentFrom) {
  GraphSnapshot src;
  src.next_id_node = 9;
  src.next_id_relation = 5;

  src.ontology_node.emplace("model", NodeType("#000000", "rect", "#000000"));
  src.ontology_relation.emplace("has_special_case", RelationType("#000000", "solid", false, true));

  const NodeId kAliveNode1 = 1;
  const NodeId kAliveNode2 = 2;
  const NodeId kAliveNode3 = 5;
  src.nodes.emplace_back(kAliveNode1, "Ряд Дирихле", "mod");
  const double size = 4.0;
  const std::pair<double, double> hand_position{4.0, 8.0};
  src.nodes.emplace_back(kAliveNode2, "Гармонический ряд", "model", "1/n", size, hand_position);
  src.nodes.emplace_back(kAliveNode3, "Телескопический ряд", "model");
  const RelationId kAliveRelation = 3;
  const NodeId kDead = 56;

  src.relations.emplace_back(kAliveRelation, kDead, kAliveNode2, "has_special_case");

  EXPECT_THROW(Graph::FromSnapshot(src), std::runtime_error);
}

TEST(GraphFromSnapshot, NextIdNodeTooBig) {
  GraphSnapshot src;
  src.next_id_node = 100000000000000000;
  src.next_id_relation = 5;

  src.ontology_node.emplace("model", NodeType("#000000", "rect", "#000000"));
  src.ontology_relation.emplace("has_special_case", RelationType("#000000", "solid", false, true));

  const NodeId kAliveNode1 = 1;
  const NodeId kAliveNode2 = 2;
  const NodeId kAliveNode3 = 5;
  src.nodes.emplace_back(kAliveNode1, "Ряд Дирихле", "mod");
  const double size = 4.0;
  const std::pair<double, double> hand_position{4.0, 8.0};
  src.nodes.emplace_back(kAliveNode2, "Гармонический ряд", "model", "1/n", size, hand_position);
  src.nodes.emplace_back(kAliveNode3, "Телескопический ряд", "model");
  const RelationId kAliveRelation = 3;
  src.relations.emplace_back(kAliveRelation, kAliveNode1, kAliveNode2, "has_special_case");

  EXPECT_THROW(Graph::FromSnapshot(src), std::runtime_error);
}

TEST(GraphFromSnapshot, RepeatedIdNode) {
  GraphSnapshot src;
  src.next_id_node = 100;
  src.next_id_relation = 5;

  src.ontology_node.emplace("model", NodeType("#000000", "rect", "#000000"));
  src.ontology_relation.emplace("has_special_case", RelationType("#000000", "solid", false, true));

  const NodeId kAliveNode1 = 1;
  const NodeId kAliveNode2 = 5;
  const NodeId kAliveNode3 = 5;
  src.nodes.emplace_back(kAliveNode1, "Ряд Дирихле", "mod");
  const double size = 4.0;
  const std::pair<double, double> hand_position{4.0, 8.0};
  src.nodes.emplace_back(kAliveNode2, "Гармонический ряд", "model", "1/n", size, hand_position);
  src.nodes.emplace_back(kAliveNode3, "Телескопический ряд", "model");
  const RelationId kAliveRelation = 3;
  src.relations.emplace_back(kAliveRelation, kAliveNode1, kAliveNode2, "has_special_case");

  EXPECT_THROW(Graph::FromSnapshot(src), std::runtime_error);
}

TEST(GraphFromSnapshot, RepeatedIdRelation) {
  GraphSnapshot src;
  src.next_id_node = 100;
  src.next_id_relation = 5;

  src.ontology_node.emplace("model", NodeType("#000000", "rect", "#000000"));
  src.ontology_relation.emplace("has_special_case", RelationType("#000000", "solid", false, true));

  const NodeId kAliveNode1 = 1;
  const NodeId kAliveNode2 = 2;
  const NodeId kAliveNode3 = 5;
  src.nodes.emplace_back(kAliveNode1, "Ряд Дирихле", "mod");
  const double size = 4.0;
  const std::pair<double, double> hand_position{4.0, 8.0};
  src.nodes.emplace_back(kAliveNode2, "Гармонический ряд", "model", "1/n", size, hand_position);
  src.nodes.emplace_back(kAliveNode3, "Телескопический ряд", "model");
  const RelationId kAliveRelation = 3;
  src.relations.emplace_back(kAliveRelation, kAliveNode1, kAliveNode2, "has_special_case");
  src.relations.emplace_back(kAliveRelation, kAliveNode1, kAliveNode3, "has_special_case");

  EXPECT_THROW(Graph::FromSnapshot(src), std::runtime_error);
}

TEST(GraphForEachLive, TryIterate) {
  Graph src;
  src.AddRelationType("has_special_case", "#000000", "solid", false, true);
  src.AddRelationType("boils_down_to", "#000000", "solid", false, true);
  src.AddNodeType("model", "#4a90d9", "rect", "#000000");
  NodeId node1 = src.AddNode("Числовой ряд", "model");
  NodeId node2 = src.AddNode("Функциональный ряд", "model");
  [[maybe_unused]] NodeId node3 = src.AddNodeWithRelations(
      "Ряд", "model", {}, {{node1, "has_special_case"}, {node2, "has_special_case"}});
  [[maybe_unused]] RelationId relation3 = src.AddRelation(node2, node1, "boils_down_to");
  NodeId node4 = src.AddNode("Признак Дирихле", "model");
  NodeId deleted5 = src.AddNode("Признак Лейбница", "model");
  NodeId node6 = src.AddNode("Признак Абеля", "model");
  [[maybe_unused]] RelationId relation4 = src.AddRelation(node1, node4, "has_special_case");
  [[maybe_unused]] RelationId deleted_rel5 = src.AddRelation(node4, deleted5, "has_special_case");
  [[maybe_unused]] RelationId relation6 = src.AddRelation(node4, node6, "has_special_case");
  NodeId deleted7 = src.AddNode("Признак Вейерштрасса", "model");
  NodeId deleted8 = src.AddNode("Признак Коши", "model");
  [[maybe_unused]] RelationId deleted_rel7 = src.AddRelation(node2, deleted7, "has_special_case");
  [[maybe_unused]] RelationId deleted_rel8 = src.AddRelation(node2, deleted8, "has_special_case");
  src.DeleteNodes({deleted5, deleted7, deleted8});

  std::vector<NodeId> live_node_ids;
  std::vector<RelationId> live_relation_ids;
  src.ForEachLiveNode([&live_node_ids](const Node& node) {
    live_node_ids.push_back(node.id());
    return true;
  });
  src.ForEachLiveRelation([&live_relation_ids](const Relation& relation) {
    live_relation_ids.push_back(relation.id());
    return true;
  });

  std::vector<NodeId> expected_live_node_id({1, 2, 3, 4, 6});
  std::vector<NodeId> expected_live_relation_id({1, 2, 3, 4, 6});
  EXPECT_EQ(live_node_ids, expected_live_node_id);
  EXPECT_EQ(live_relation_ids, expected_live_relation_id);

  const NodeId search_for_id = 4;
  Node search_for(0, "", "");
  src.ForEachLiveNode([&search_for_id, &search_for](const Node& node) {
    if (search_for_id == node.id()) {
      search_for = node;
      return false;
    } else {
      return true;
    }
  });
  EXPECT_EQ(search_for.id(), search_for_id);
  EXPECT_EQ(search_for.title(), "Признак Дирихле");
  EXPECT_EQ(search_for.type(), "model");

  const RelationId search_for_id_r = 4;
  Relation search_for_r(0, 0, 0, "");
  src.ForEachLiveRelation([&search_for_id_r, &search_for_r](const Relation& relation) {
    if (search_for_id_r == relation.id()) {
      search_for_r = relation;
      return false;
    } else {
      return true;
    }
  });
  EXPECT_EQ(search_for_r.id(), search_for_id_r);
  EXPECT_EQ(search_for_r.from(), 1);
  EXPECT_EQ(search_for_r.to(), 4);
  EXPECT_EQ(search_for_r.type(), "has_special_case");
}

TEST(GraphToSnapshot, ValidGraphSnapshotWithHoles) {
  Graph src;
  src.AddRelationType("has_special_case", "#000000", "solid", false, true);
  src.AddRelationType("boils_down_to", "#000000", "solid", false, true);
  src.AddNodeType("model", "#4a90d9", "rect", "#000000");
  NodeId node1 = src.AddNode("Числовой ряд", "model");
  NodeId node2 = src.AddNode("Функциональный ряд", "model");
  NodeId node3 = src.AddNodeWithRelations(
      "Ряд", "model", {}, {{node1, "has_special_case"}, {node2, "has_special_case"}});
  RelationId relation3 = src.AddRelation(node2, node1, "boils_down_to");
  NodeId node4 = src.AddNode("Признак Дирихле", "model");
  NodeId deleted5 = src.AddNode("Признак Лейбница", "model");
  NodeId node6 = src.AddNode("Признак Абеля", "model");
  RelationId relation4 = src.AddRelation(node1, node4, "has_special_case");
  [[maybe_unused]] RelationId deleted_rel5 = src.AddRelation(node4, deleted5, "has_special_case");
  RelationId relation6 = src.AddRelation(node4, node6, "has_special_case");
  NodeId deleted7 = src.AddNode("Признак Вейерштрасса", "model");
  NodeId deleted8 = src.AddNode("Признак Коши", "model");
  [[maybe_unused]] RelationId deleted_rel7 = src.AddRelation(node2, deleted7, "has_special_case");
  [[maybe_unused]] RelationId deleted_rel8 = src.AddRelation(node2, deleted8, "has_special_case");
  src.DeleteNodes({deleted5, deleted7, deleted8});

  NodeId right_next_id_node = src.next_id_node();
  NodeId right_next_id_relation = src.next_id_relation();
  std::unordered_map<std::string, NodeType> right_ontology_node = src.ontology_node();
  std::unordered_map<std::string, RelationType> right_ontology_relation = src.ontology_relation();
  std::vector<Node> right_nodes;
  right_nodes.emplace_back(node1, "Числовой ряд", "model");
  right_nodes.emplace_back(node2, "Функциональный ряд", "model");
  right_nodes.emplace_back(node3, "Ряд", "model");
  right_nodes.emplace_back(node4, "Признак Дирихле", "model");
  right_nodes.emplace_back(node6, "Признак Абеля", "model");
  std::vector<Relation> right_relations;
  right_relations.emplace_back(1, node3, node1, "has_special_case");
  right_relations.emplace_back(2, node3, node2, "has_special_case");
  right_relations.emplace_back(relation3, node2, node1, "boils_down_to");
  right_relations.emplace_back(relation4, node1, node4, "has_special_case");
  right_relations.emplace_back(relation6, node4, node6, "has_special_case");

  GraphSnapshot dest = src.ToSnapshot();
  EXPECT_EQ(dest.next_id_node, right_next_id_node);
  EXPECT_EQ(dest.next_id_relation, right_next_id_relation);

  EXPECT_EQ(dest.ontology_node, right_ontology_node);
  EXPECT_EQ(dest.ontology_relation, right_ontology_relation);

  EXPECT_EQ(dest.nodes, right_nodes);
  EXPECT_EQ(dest.relations, right_relations);
}

TEST(GraphDeleteRelations, DuplicateRelationIdsInArgument) {
  // Контракт идемпотентности (implementation.md): дубликат id в списке на удаление —
  // вырожденный случай корректного запроса, не ошибка.
  Graph g;
  g.AddRelationType("has_special_case", "#000000", "solid", false, true);
  g.AddNodeType("model", "#4a90d9", "rect", "#000000");
  NodeId from = g.AddNode("Числовой ряд", "model");
  NodeId to = g.AddNode("Функциональный ряд", "model");
  const RelationId id1 = g.AddRelation(from, to, "has_special_case");
  const RelationId id2 = g.AddRelation(to, from, "has_special_case");

  g.DeleteRelations({id1, id1, id2});  // не бросает, состояние корректно

  EXPECT_FALSE(g.HasRelation(id1));
  EXPECT_FALSE(g.HasRelation(id2));
  EXPECT_TRUE(g.GetIncidentRelations(from).empty());
  EXPECT_TRUE(g.GetIncidentRelations(to).empty());
  EXPECT_TRUE(g.HasNode(from) && g.HasNode(to));  // узлы не пострадали

  NodeId big = g.AddNode("Ряд", "model");
  const RelationId deleted = g.AddRelation(big, to, "has_special_case");

  g.DeleteRelations({deleted});
  EXPECT_THROW(g.DeleteRelations({deleted}), std::invalid_argument);
}

TEST(GraphDeleteNodes, DuplicateNodeIdsInArgument) {
  // Тот же контракт идемпотентности для DeleteNodes: повтор id удаляет один раз.
  Graph g;
  g.AddRelationType("has_special_case", "#000000", "solid", false, true);
  g.AddNodeType("model", "#4a90d9", "rect", "#000000");
  NodeId from = g.AddNode("Числовой ряд", "model");
  NodeId to = g.AddNode("Функциональный ряд", "model");
  NodeId big = g.AddNode("Ряд", "model");
  const RelationId id = g.AddRelation(from, to, "has_special_case");

  g.DeleteNodes({from, from});  // не бросает

  EXPECT_FALSE(g.HasNode(from));
  EXPECT_TRUE(g.HasNode(to));
  EXPECT_FALSE(g.HasRelation(id));
  EXPECT_TRUE(g.GetIncidentRelations(to).empty());

  g.DeleteNodes({big});
  EXPECT_THROW(g.DeleteNodes({big}), std::invalid_argument);
}

TEST(GraphFromSnapshot, IdBeyondNextIdUnsorted) {
  // Правило 22: вход полный и валидный ровно в одном месте — узел id 7 при
  // next_id_node = 4. Порядок [1, 7, 3]: старая проверка через back() (id 3 < 4)
  // пропускала этот файл и доходила до записи за пределами вектора (UB).
  GraphSnapshot src;
  src.next_id_node = 4;
  src.next_id_relation = 2;

  src.ontology_node.emplace("model", NodeType("#000000", "rect", "#000000"));
  src.ontology_relation.emplace("has_special_case", RelationType("#000000", "solid", false, true));

  src.nodes.emplace_back(1, "Ряд Дирихле", "model");
  src.nodes.emplace_back(7, "Гармонический ряд", "model");
  src.nodes.emplace_back(3, "Телескопический ряд", "model");
  src.relations.emplace_back(1, 1, 3, "has_special_case");

  EXPECT_THROW(Graph::FromSnapshot(src), std::runtime_error);
}

TEST(GraphFromSnapshot, UnsortedOrderAccepted) {
  // Порядок id в снимке не контракт (R11, ToSnapshot пишет в порядке access_):
  // несортированный, но валидный снимок обязан грузиться.
  GraphSnapshot src;
  src.next_id_node = 4;
  src.next_id_relation = 2;

  src.ontology_node.emplace("model", NodeType("#000000", "rect", "#000000"));
  src.ontology_relation.emplace("has_special_case", RelationType("#000000", "solid", false, true));

  src.nodes.emplace_back(3, "Телескопический ряд", "model");
  src.nodes.emplace_back(1, "Ряд Дирихле", "model");
  src.nodes.emplace_back(2, "Гармонический ряд", "model");
  src.relations.emplace_back(1, 1, 2, "has_special_case");

  Graph g = Graph::FromSnapshot(src);
  EXPECT_TRUE(g.HasNode(1) && g.HasNode(2) && g.HasNode(3));
  EXPECT_EQ(g.GetNode(1).title(), "Ряд Дирихле");
  EXPECT_EQ(g.GetNode(3).title(), "Телескопический ряд");
  EXPECT_TRUE(g.HasRelation(1));
  EXPECT_EQ(g.next_id_node(), 4);
}

TEST(GraphRestoreNodes, ToRessurectNodes) {
  Graph src;
  src.AddRelationType("has_special_case", "#000000", "solid", false, true);
  src.AddRelationType("boils_down_to", "#000000", "solid", false, true);
  src.AddNodeType("model", "#4a90d9", "rect", "#000000");
  NodeId node1 = src.AddNode("Числовой ряд", "model");
  NodeId node2 = src.AddNode("Функциональный ряд", "model");
  [[maybe_unused]] NodeId node3 = src.AddNodeWithRelations(
      "Ряд", "model", {}, {{node1, "has_special_case"}, {node2, "has_special_case"}});
  [[maybe_unused]] RelationId relation3 = src.AddRelation(node2, node1, "boils_down_to");
  NodeId node4 = src.AddNode("Признак Дирихле", "model");
  NodeId deleted5 = src.AddNode("Признак Лейбница", "model");
  NodeId node6 = src.AddNode("Признак Абеля", "model");
  [[maybe_unused]] RelationId relation4 = src.AddRelation(node1, node4, "has_special_case");
  RelationId deleted_rel5 = src.AddRelation(node4, deleted5, "has_special_case");
  [[maybe_unused]] RelationId relation6 = src.AddRelation(node4, node6, "has_special_case");
  NodeId deleted7 = src.AddNode("Признак Вейерштрасса", "model");
  NodeId deleted8 = src.AddNode("Признак Коши", "model");
  [[maybe_unused]] RelationId deleted_rel7 = src.AddRelation(node2, deleted7, "has_special_case");
  [[maybe_unused]] RelationId deleted_rel8 = src.AddRelation(node2, deleted8, "has_special_case");

  Graph dest(src);

  dest.DeleteNodes({deleted5, deleted7, deleted8});
  EXPECT_FALSE(dest == src);
  EXPECT_THROW(dest.DeleteNodes({deleted5}), std::invalid_argument);
  dest.RestoreNodes({deleted5, deleted7, deleted8});
  dest.RestoreRelations({deleted_rel5, deleted_rel7, deleted_rel8});
  EXPECT_TRUE(dest == src);
}

TEST(GraphRestoreNodes, ToRessurectRelations) {
  Graph src;
  src.AddRelationType("has_special_case", "#000000", "solid", false, true);
  src.AddRelationType("boils_down_to", "#000000", "solid", false, true);
  src.AddNodeType("model", "#4a90d9", "rect", "#000000");
  NodeId node1 = src.AddNode("Числовой ряд", "model");
  NodeId node2 = src.AddNode("Функциональный ряд", "model");
  [[maybe_unused]] NodeId node3 = src.AddNodeWithRelations(
      "Ряд", "model", {}, {{node1, "has_special_case"}, {node2, "has_special_case"}});
  [[maybe_unused]] RelationId relation3 = src.AddRelation(node2, node1, "boils_down_to");
  NodeId node4 = src.AddNode("Признак Дирихле", "model");
  NodeId node5 = src.AddNode("Признак Лейбница", "model");
  NodeId node6 = src.AddNode("Признак Абеля", "model");
  [[maybe_unused]] RelationId relation4 = src.AddRelation(node1, node4, "has_special_case");
  RelationId deleted_rel5 = src.AddRelation(node4, node5, "has_special_case");
  [[maybe_unused]] RelationId relation6 = src.AddRelation(node4, node6, "has_special_case");
  NodeId node7 = src.AddNode("Признак Вейерштрасса", "model");
  NodeId node8 = src.AddNode("Признак Коши", "model");
  [[maybe_unused]] RelationId deleted_rel7 = src.AddRelation(node2, node7, "has_special_case");
  [[maybe_unused]] RelationId deleted_rel8 = src.AddRelation(node2, node8, "has_special_case");

  Graph dest(src);

  dest.DeleteRelations({deleted_rel5, deleted_rel7, deleted_rel8});
  EXPECT_FALSE(dest == src);
  EXPECT_THROW(dest.DeleteRelations({deleted_rel7}), std::invalid_argument);
  dest.RestoreRelations({deleted_rel5, deleted_rel7, deleted_rel8});
  EXPECT_TRUE(dest == src);
}

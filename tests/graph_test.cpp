#pragma warning(disable : 26495 26439)

#include "graph.hpp"

#include <gtest/gtest.h>

using ravel::Graph;
using ravel::GraphSnapshot;
using ravel::NodeId;
using ravel::NodeType;
using ravel::RelationId;
using ravel::RelationType;
using ravel::Node;
using ravel::Relation;

TEST(GraphAddNode, CreatesAccessEntryAndBumpsCounter) {
  Graph g;
  g.AddNodeType("model", "#4a90d9", "rect", "#000000");
  const NodeId id = g.AddNode("Вещественная прямая", "model");
  EXPECT_THROW(g.AddNode("Сложение", "function"),
               std::out_of_range);  // инвариант 5: тип есть в словаре
  EXPECT_TRUE(g.HasNode(id));       // инвариант 3: запись в access_
  EXPECT_EQ(g.GetNode(id).Type(), "model");
  EXPECT_EQ(g.NextIdNode(), id + 1);  // инвариант 1: счётчик выдал id
}

TEST(GraphAddRelation, CreatesAccessEntryCounter) {
  Graph g;
  g.AddRelationType("has_special_case", "#3ba895", "solid", false, true);
  g.AddNodeType("model", "#4a90d9", "rect", "#000000");
  NodeId from = g.AddNode("Признак Дирихле", "model");
  NodeId to = g.AddNode("Признак Лейбница", "model");
  const RelationId id = g.AddRelation(from, to, "has_special_case");

  EXPECT_THROW(g.AddRelation(from, 3, "has_special_case"),
               std::out_of_range);  // инваринт 4: from и to существуют
  EXPECT_THROW(g.AddRelation(5, to, "has_special_case"),
               std::out_of_range);  // инваринт 4: from и to существуют
  EXPECT_THROW(g.AddRelation(from, to, "is_a"),
               std::out_of_range);  // инвариант  5: тип есть в словаре

  EXPECT_EQ(std::vector<RelationId>{id},
            g.GetIncidentRelations(from));  // инвариант 3: запись в access_
  EXPECT_EQ(std::vector<RelationId>{id},
            g.GetIncidentRelations(to));  // инвариант 3: запись в access_
  EXPECT_EQ(g.NextIdRelation(), id + 1);  // инвариант 1: счётчик выдал id
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
  EXPECT_EQ(std::set<NodeId>{}, g.GetNeighbors(to1));  // инвариант 3: запись в access_
  EXPECT_EQ(std::set<NodeId>{}, g.GetNeighbors(to2));  // инвариант 3: запись в access_
  EXPECT_EQ(g.NextIdNode(), to2 + 1);                  // инвариант 1: id не переиспользуются
}

TEST(GraphMoveNode, SetNewHandPosition) {
  Graph g;
  g.AddNodeType("model", "#4a90d9", "rect", "#000000");
  NodeId id =
      g.AddNode("Датчик Холла", "model", "лаба по физике", 1, std::pair<double, double>{100, 100});
  std::optional<std::pair<double, double>> before = g.GetNode(id).HandPosition().value();
  EXPECT_EQ(before.value(), std::make_pair(100.0, 100.0));
  std::optional<std::pair<double, double>> prev = g.MoveNode(id, {-100, -100});
  EXPECT_EQ(prev.value(), std::make_pair(100.0, 100.0));
  EXPECT_TRUE(g.HasNode(id));
  EXPECT_EQ(g.GetNode(id).HandPosition().value(), std::make_pair(-100.0, -100.0));

  NodeId temp = g.AddNode("Коаксильный кабель", "model");
  g.DeleteNodes({temp});
  EXPECT_THROW(g.MoveNode(temp, {-100, 100}), std::out_of_range);

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
  EXPECT_EQ(g.NextIdNode(), test + 1);

  const size_t kAddedRelations = 3;

  EXPECT_EQ(g.NextIdRelation(), kAddedRelations + 1);

  EXPECT_THROW(
      g.AddNodeWithRelations("Однородные уравнения", "modl",
                             {{from1, "reduce_to"}, {from2, "reduce_to"}}, {{to1, "reduce_to"}}),
      std::out_of_range);
  EXPECT_EQ(g.NextIdNode(), test + 1);
  EXPECT_EQ(g.NextIdRelation(), kAddedRelations + 1);

  EXPECT_THROW(
      g.AddNodeWithRelations("Однородные уравнения", "model",
                             {{from1, "reduc_to"}, {from2, "reduce_to"}}, {{to1, "reduce_to"}}),
      std::out_of_range);
  EXPECT_EQ(g.NextIdNode(), test + 1);
  EXPECT_EQ(g.NextIdRelation(), kAddedRelations + 1);
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
  EXPECT_EQ(g.NextIdNode(), src.next_id_node);
  EXPECT_EQ(g.NextIdRelation(), src.next_id_relation);

  std::unordered_map<std::string, NodeType> right_node_types;
  right_node_types.emplace("model", NodeType("#000000", "rect", "#000000"));
  EXPECT_EQ(g.OntologyNode(), right_node_types);

  std::unordered_map<std::string, RelationType> right_relation_types;
  right_relation_types.emplace("has_special_case", RelationType("#000000", "solid", false, true));
  EXPECT_EQ(g.OntologyRelation(), right_relation_types);

  for (NodeId id = 0; id < src.next_id_node; ++id) {
    if (id == kAliveNode1 || id == kAliveNode2 || id == kAliveNode3) {
      EXPECT_TRUE(g.HasNode(id));
    } else {
      EXPECT_FALSE(g.HasNode(id));
    }
  }
  EXPECT_EQ(g.GetNode(kAliveNode1).Id(), kAliveNode1);
  EXPECT_EQ(g.GetNode(kAliveNode1).Title(), "Ряд Дирихле");
  EXPECT_EQ(g.GetNode(kAliveNode1).Type(), "model");
  EXPECT_EQ(g.GetNode(kAliveNode2).Id(), kAliveNode2);
  EXPECT_EQ(g.GetNode(kAliveNode2).Title(), "Гармонический ряд");
  EXPECT_EQ(g.GetNode(kAliveNode2).Type(), "model");
  EXPECT_EQ(g.GetNode(kAliveNode2).Description(), "1/n");
  EXPECT_EQ(g.GetNode(kAliveNode2).Size(), size);
  EXPECT_EQ(g.GetNode(kAliveNode2).HandPosition(), hand_position);
  EXPECT_EQ(g.GetNode(kAliveNode3).Id(), kAliveNode3);
  EXPECT_EQ(g.GetNode(kAliveNode3).Title(), "Телескопический ряд");
  EXPECT_EQ(g.GetNode(kAliveNode3).Type(), "model");

  for (RelationId id = 0; id < src.next_id_relation; ++id) {
    if (id == kAliveRelation) {
      EXPECT_TRUE(g.HasRelation(id));
    } else {
      EXPECT_FALSE(g.HasRelation(id));
    }
  }
  EXPECT_EQ(g.GetRelation(kAliveRelation).From(), kAliveNode1);
  EXPECT_EQ(g.GetRelation(kAliveRelation).To(), kAliveNode2);
  EXPECT_EQ(g.GetRelation(kAliveRelation).Description(), "");
  EXPECT_EQ(g.GetRelation(kAliveRelation).Type(), "has_special_case");

  EXPECT_EQ(g.GetIncidentRelations(kAliveNode1), std::vector<RelationId>({kAliveRelation}));
  EXPECT_EQ(g.GetIncidentRelations(kAliveNode2), std::vector<RelationId>({kAliveRelation}));
  EXPECT_EQ(g.GetIncidentRelations(kAliveNode3), std::vector<RelationId>());

  EXPECT_EQ(g.GetNeighbors(kAliveNode1), std::set<NodeId>({kAliveNode2}));
  EXPECT_EQ(g.GetNeighbors(kAliveNode2), std::set<NodeId>({kAliveNode1}));
  EXPECT_EQ(g.GetNeighbors(kAliveNode3), std::set<NodeId>());
}

TEST(GraphFromSnaphot, EmptySnaphot) {
  GraphSnapshot src;
  Graph g = Graph::FromSnapshot(src);
  EXPECT_EQ(g.NextIdNode(), 1);
  EXPECT_EQ(g.NextIdRelation(), 1);
  const std::unordered_map<std::string, NodeType> empty_node_type;
  EXPECT_EQ(g.OntologyNode(), empty_node_type);
  const std::unordered_map<std::string, RelationType> empty_relation_type;
  EXPECT_EQ(g.OntologyRelation(), empty_relation_type);
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

TEST(GraphToSnapshot, ValidGraphSnapshotWithdHoles) {
  Graph src;
  src.AddRelationType("has_special_case", "#000000", "solid", false, true);
  src.AddRelationType("boils_down_to", "#000000", "solid", false, true);
  src.AddNodeType("model", "#4a90d9", "rect", "#000000");
  NodeId node1 = src.AddNode("Числовой ряд", "model");
  NodeId node2 = src.AddNode("Функциональный ряд", "model");
  NodeId node3= src.AddNodeWithRelations(
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

  NodeId right_next_id_node = src.NextIdNode();
  NodeId right_next_id_relation = src.NextIdRelation();
  std::unordered_map<std::string, NodeType> right_ontology_node = src.OntologyNode();
  std::unordered_map<std::string, RelationType> right_ontology_relation = src.OntologyRelation();
  std::vector<Node> right_nodes;
  right_nodes.emplace_back(node1, "Числовой ряд", "model");
  right_nodes.emplace_back(node2, "Функциональный ряд", "model");
  right_nodes.emplace_back(node3, "Ряд", "model");
  right_nodes.emplace_back(node4, "Признак Дирихле", "model");
  right_nodes.emplace_back(node6, "Признак Абеля", "model");
  std::vector<Relation> right_relations;
  right_relations.emplace_back(1, node3, node1, "has_special_case");
  right_relations.emplace_back(2, node3, node2, "has_special_case");
  right_relations.emplace_back(relation3, node2, node1 , "boils_down_to");
  right_relations.emplace_back(relation4, node1, node4 , "has_special_case");
  right_relations.emplace_back(relation6, node4, node6 , "has_special_case");

  GraphSnapshot dest = src.ToSnapshot();
  EXPECT_EQ(dest.next_id_node, right_next_id_node);
  EXPECT_EQ(dest.next_id_relation, right_next_id_relation);

  EXPECT_EQ(dest.ontology_node, right_ontology_node);
  EXPECT_EQ(dest.ontology_relation, right_ontology_relation);

  EXPECT_EQ(dest.nodes, right_nodes);
  EXPECT_EQ(dest.relations, right_relations);
}
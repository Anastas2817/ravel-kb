#include <gtest/gtest.h>
#include "graph.hpp"

TEST(GraphAddNode, CreatesAccessEntryAndBumpsCounter) {
  Graph g;
  g.AddNodeType("model", "#4a90d9", "rect", "#000000");
  const NodeId id = g.AddNode("Вещественная прямая", "model");
  EXPECT_TRUE(g.HasNode(id));              // инвариант 3: запись в access_
  EXPECT_EQ(g.GetNode(id).Type(), "model");
  EXPECT_EQ(g.NextIdNode(), id + 1);  // инвариант 1: счётчик выдал id
}
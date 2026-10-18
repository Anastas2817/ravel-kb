#include "command.hpp"

#include <optional>
#include <utility>

#include <gtest/gtest.h>

namespace {

ravel::Graph MakeGraphWithTypes() {
  ravel::Graph g;
  g.AddNodeType("concept", "#112233", "rect", "#445566");
  g.AddRelationType("is_a", "#112233", "solid", true, true);
  return g;
}

}  // namespace

TEST(AddNodeCmd, DoUndoRedo) {
  ravel::Graph g = MakeGraphWithTypes();
  ravel::app::CommandChronicle chronicle;

  ravel::app::AddNodeCmd cmd{.title = "Title", .type = "concept", .description = ""};
  ASSERT_TRUE(chronicle.Execute(cmd, g).has_value());
  ASSERT_TRUE(g.HasNode(1));

  ASSERT_TRUE(chronicle.Undo(g).has_value());
  EXPECT_FALSE(g.HasNode(1));

  // Redo restores the same entity (soft-delete), not a new one.
  ASSERT_TRUE(chronicle.Redo(g).has_value());
  ASSERT_TRUE(g.HasNode(1));
  EXPECT_EQ(g.GetNode(1).title(), "Title");
}

TEST(AddRelationCmd, DoUndoRedo) {
  ravel::Graph g = MakeGraphWithTypes();
  const ravel::NodeId from = g.AddNode("A", "concept");
  const ravel::NodeId to = g.AddNode("B", "concept");
  ravel::app::CommandChronicle chronicle;

  ravel::app::AddRelationCmd cmd{.from = from, .to = to, .type = "is_a", .description = ""};
  ASSERT_TRUE(chronicle.Execute(cmd, g).has_value());
  ASSERT_TRUE(g.HasRelation(1));

  ASSERT_TRUE(chronicle.Undo(g).has_value());
  EXPECT_FALSE(g.HasRelation(1));

  ASSERT_TRUE(chronicle.Redo(g).has_value());
  ASSERT_TRUE(g.HasRelation(1));
  EXPECT_EQ(g.GetRelation(1).from(), from);
  EXPECT_EQ(g.GetRelation(1).to(), to);
}

TEST(DeleteNodesCmd, DoUndoRedo) {
  ravel::Graph g = MakeGraphWithTypes();
  const ravel::NodeId a = g.AddNode("A", "concept");
  const ravel::NodeId b = g.AddNode("B", "concept");
  const ravel::NodeId c = g.AddNode("C", "concept");
  const ravel::RelationId ab = g.AddRelation(a, b, "is_a");
  const ravel::RelationId bc = g.AddRelation(b, c, "is_a");
  const ravel::Graph before = g;
  ravel::app::CommandChronicle chronicle;

  ravel::app::DeleteNodesCmd cmd{{b}, {}};
  ASSERT_TRUE(chronicle.Execute(cmd, g).has_value());
  EXPECT_FALSE(g.HasNode(b));
  EXPECT_FALSE(g.HasRelation(ab));  // cascade
  EXPECT_FALSE(g.HasRelation(bc));
  EXPECT_TRUE(g.HasNode(a));
  EXPECT_TRUE(g.HasNode(c));

  // Undo must resurrect the cascade too: deleted_relations captured it.
  ASSERT_TRUE(chronicle.Undo(g).has_value());
  EXPECT_TRUE(g.SessionEqual(before));

  ASSERT_TRUE(chronicle.Redo(g).has_value());
  EXPECT_FALSE(g.HasNode(b));
  EXPECT_FALSE(g.HasRelation(ab));
  EXPECT_FALSE(g.HasRelation(bc));
}

TEST(DeleteRelationsCmd, DoUndoRedo) {
  ravel::Graph g = MakeGraphWithTypes();
  const ravel::NodeId from = g.AddNode("A", "concept");
  const ravel::NodeId to = g.AddNode("B", "concept");
  const ravel::RelationId rid = g.AddRelation(from, to, "is_a");
  const ravel::Graph before = g;
  ravel::app::CommandChronicle chronicle;

  ravel::app::DeleteRelationsCmd cmd{{rid}};
  ASSERT_TRUE(chronicle.Execute(cmd, g).has_value());
  EXPECT_FALSE(g.HasRelation(rid));
  EXPECT_TRUE(g.HasNode(from));
  EXPECT_TRUE(g.HasNode(to));

  ASSERT_TRUE(chronicle.Undo(g).has_value());
  EXPECT_TRUE(g.SessionEqual(before));

  ASSERT_TRUE(chronicle.Redo(g).has_value());
  EXPECT_FALSE(g.HasRelation(rid));
}

TEST(MoveNodeCmd, DoUndoRedo) {
  ravel::Graph g = MakeGraphWithTypes();
  const ravel::NodeId a = g.AddNode("A", "concept", "", 1);
  g.MoveNode(a, ravel::Position{1.0, 2.0});
  ravel::app::CommandChronicle chronicle;

  ravel::app::MoveNodeCmd cmd{a, {0.0, 0.0}, {3.5, 4.5}};
  ASSERT_TRUE(chronicle.Execute(cmd, g).has_value());
  EXPECT_EQ(g.GetPosition(a), std::make_optional(ravel::Position{3.5, 4.5}));

  ASSERT_TRUE(chronicle.Undo(g).has_value());
  EXPECT_EQ(g.GetPosition(a), std::make_optional(ravel::Position{1.0, 2.0}));

  ASSERT_TRUE(chronicle.Redo(g).has_value());
  EXPECT_EQ(g.GetPosition(a), std::make_optional(ravel::Position{3.5, 4.5}));
}

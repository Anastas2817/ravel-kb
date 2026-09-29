#pragma warning(disable : 26495 26439)

#include "storage.hpp"

#include <gtest/gtest.h>

using ravel::Graph;
using ravel::JsonStorage;
using ravel::NodeId;
using ravel::RelationId;

TEST(StorageLoadSave, RoundTrip) {
  Graph src;
  src.AddNodeType("model", "#002030", "rect", "#a0500b");
  src.AddRelationType("reduce_to", "#003d00", "dashed", false, true);

  NodeId to1 = src.AddNode("Уравнения с разделяющимися переменными", "model", "f(x)dx=g(y)dy", 2,
                           std::make_pair(2.0, 3.0));
  NodeId from1 = src.AddNode("Уравнения, сводящиеся к однородным", "model");
  NodeId from2 = src.AddNode("Уравнения Бернулли", "model");
  src.AddNodeWithRelations("Однородные уравнения", "model",
                           {{from1, "reduce_to"}, {from2, "reduce_to"}}, {{to1, "reduce_to"}});

  std::string saved = JsonStorage::Save(src);
  Graph dest = JsonStorage::Load(saved);

  EXPECT_EQ(src.NextIdNode(), dest.NextIdNode());
  EXPECT_EQ(src.NextIdRelation(), dest.NextIdRelation());

  EXPECT_EQ(dest.ResolveNode("model").Color(), src.ResolveNode("model").Color());
  EXPECT_EQ(dest.ResolveNode("model").FrameColor(), src.ResolveNode("model").FrameColor());
  EXPECT_EQ(dest.ResolveNode("model").Shape(), src.ResolveNode("model").Shape());
  EXPECT_EQ(dest.ResolveRelation("reduce_to").Arrow(), src.ResolveRelation("reduce_to").Arrow());
  EXPECT_EQ(dest.ResolveRelation("reduce_to").Color(), src.ResolveRelation("reduce_to").Color());
  EXPECT_EQ(dest.ResolveRelation("reduce_to").Symmetric(),
            src.ResolveRelation("reduce_to").Symmetric());
  EXPECT_EQ(dest.ResolveRelation("reduce_to").Transitive(),
            src.ResolveRelation("reduce_to").Transitive());

  for (NodeId id = 1; id < dest.NextIdNode(); ++id) {  // предусловие строка 17
    if (!dest.HasNode(id)) {
      EXPECT_FALSE(src.HasNode(id));
    } else {
      EXPECT_EQ(dest.GetNode(id).Description(), src.GetNode(id).Description());
      EXPECT_EQ(dest.GetNode(id).HandPosition(), src.GetNode(id).HandPosition());
      EXPECT_EQ(dest.GetNode(id).Size(), src.GetNode(id).Size());
      EXPECT_EQ(dest.GetNode(id).Title(), src.GetNode(id).Title());
      EXPECT_EQ(dest.GetNode(id).Type(), src.GetNode(id).Type());

      EXPECT_EQ(dest.GetNeighbors(id), src.GetNeighbors(id));
      EXPECT_EQ(dest.GetIncidentRelations(id), src.GetIncidentRelations(id));
    }
  }

  for (RelationId id = 1; id < dest.NextIdRelation(); ++id) {  // предусловие строка 18
    if (!dest.HasRelation(id)) {
      EXPECT_FALSE(src.HasRelation(id));
    } else {
      EXPECT_EQ(dest.GetRelation(id).From(), src.GetRelation(id).From());
      EXPECT_EQ(dest.GetRelation(id).To(), src.GetRelation(id).To());
      EXPECT_EQ(dest.GetRelation(id).Description(), src.GetRelation(id).Description());
      EXPECT_EQ(dest.GetRelation(id).Type(), src.GetRelation(id).Type());
    }
  }

  EXPECT_EQ(JsonStorage::Save(JsonStorage::Load(JsonStorage::Save(src))), JsonStorage::Save(src));
}

/*
TEST(StorageLoadSave, ValidEmpty) {
  Graph src;
  std::string saved = JsonStorage::Save(src);
  Graph dest = JsonStorage::Load(saved);

  const NodeId kStandartIdValue = 1; // implementation: R1/R2
  EXPECT_EQ(dest.NextIdNode(), kStandartIdValue);
  EXPECT_EQ(dest.NextIdRelation(), kStandartIdValue);
  EXPECT_EQ(JsonStorage::Save(JsonStorage::Load(JsonStorage::Save(src))), JsonStorage::Save(src));
}

TEST(StorageLoadSave, SurvivalOfLoners) {
  Graph src;
  src.AddNodeType("model", "#000000", "rect", "#000000");
  src.AddRelationType("is_a", "#000000", "solid", false, true);
  // ...
}

TEST(StorageLoadSave, NotSaveDeleted) {
  // ...
}

TEST(StorageLoadSave, SameHandPosition) {
  // ...
}
*/
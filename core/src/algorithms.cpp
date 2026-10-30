#include "algorithms.hpp"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <limits>
#include <ranges>
#include <string>

namespace ravel {
namespace {

enum class Phase : std::uint8_t { Enter, Scan, Exit };
enum class VColor : std::uint8_t { Unvisited, Entered, Exited };

constexpr NodeId kNoParent = std::numeric_limits<NodeId>::max();

struct Frame {
  NodeId v;
  size_t next;
  Phase phase;
};

struct Edge {
  NodeId from;
  NodeId to;
};

// Adjacency of the subgraph induced by one relation type: column `from`
// lists the `to` endpoints of its live relations of that type.
std::vector<std::vector<NodeId>> BuildAdjacency(const Graph& g, const std::string& key) {
  std::vector<std::vector<NodeId>> adj(g.next_id_node());
  NodeId max = 0;
  g.ForEachLiveRelation([&key, &adj, &max](const Relation& rel) {
    if (rel.type() == key) {
      adj[rel.from()].push_back(rel.to());
      max = std::max(max, rel.from());
      max = std::max(max, rel.to());
    }
    return true;
  });
  adj.resize(max + 1);
  return adj;
}

// Resolves every path edge into candidate relation ids and appends one
// CycleHint per combination: parallel relations of the same type
// materialize into separate hints.
void EmitHints(const Graph& g, const std::string& key, const std::vector<Edge>& path,
               NodeId back_from, NodeId back_to, std::vector<CycleHint>& result) {
  std::vector<std::vector<RelationId>> choices;
  choices.reserve(path.size() + 1);
  for (const Edge& e : path) {
    choices.push_back(g.ResolveRelationIds(e.from, e.to, key));
  }
  choices.push_back(g.ResolveRelationIds(back_from, back_to, key));

  std::vector<RelationId> combo(choices.size());
  const std::function<void(size_t)> build = [&](size_t i) {
    if (i == choices.size()) {
      result.push_back(CycleHint{key, combo});
      return;
    }
    for (const RelationId id : choices[i]) {
      combo[i] = id;
      build(i + 1);
    }
  };
  build(0);
}

// Back edge (v -> u) with u on the current DFS stack: reconstruct the chain
// of tree edges from u up to v, reversed into traversal order, and let the
// closing edge go last.
void HandleBackEdge(const Graph& g, const std::string& key, NodeId v, NodeId u,
                    const std::vector<NodeId>& parent, std::vector<CycleHint>& result) {
  std::vector<Edge> path;
  for (NodeId cur = v; cur != u; cur = parent[cur]) {
    path.push_back({parent[cur], cur});
  }
  std::reverse(path.begin(), path.end());
  EmitHints(g, key, path, v, u, result);
}

// Iterative DFS over one type-subgraph; back edges become cycle hints.
void ExploreFrom(const Graph& g, const std::string& key,
                 const std::vector<std::vector<NodeId>>& adj, NodeId s, std::vector<VColor>& color,
                 std::vector<NodeId>& parent, std::vector<CycleHint>& result) {
  std::vector<Frame> stack;
  stack.reserve(adj.size());
  stack.push_back({s, 0, Phase::Enter});

  while (!stack.empty()) {
    Frame& f = stack.back();
    switch (f.phase) {
      case Phase::Enter:
        color[f.v] = VColor::Entered;
        f.phase = Phase::Scan;
        break;

      case Phase::Scan:
        if (f.next >= adj[f.v].size()) {
          f.phase = Phase::Exit;
          break;
        }
        if (const NodeId u = adj[f.v][f.next++]; color[u] == VColor::Entered) {
          HandleBackEdge(g, key, f.v, u, parent, result);
        } else if (color[u] == VColor::Unvisited) {
          parent[u] = f.v;
          stack.push_back({u, 0, Phase::Enter});
        }
        break;

      case Phase::Exit:
        color[f.v] = VColor::Exited;
        stack.pop_back();
        break;
    }
  }
}

}  // namespace

std::vector<CycleHint> FindCycleHints(const Graph& g) {
  std::vector<CycleHint> result;
  for (const std::string& key : g.ontology_relation() | std::views::keys) {
    const std::vector<std::vector<NodeId>> adj = BuildAdjacency(g, key);
    const NodeId n = static_cast<NodeId>(adj.size());
    std::vector<VColor> color(n, VColor::Unvisited);
    std::vector<NodeId> parent(n, kNoParent);
    for (NodeId s = 0; s < n; ++s) {
      if (color[s] == VColor::Unvisited) {
        ExploreFrom(g, key, adj, s, color, parent, result);
      }
    }
  }
  return result;
}

}  // namespace ravel

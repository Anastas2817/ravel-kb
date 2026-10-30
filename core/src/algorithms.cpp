#include "algorithms.hpp"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <limits>
#include <ranges>
#include <string>

namespace ravel {

std::vector<CycleHint> FindCycleHints(const Graph& g) {
  std::vector<CycleHint> result;

  enum class Phase { Enter, Scan, Exit };
  enum class VColor { Unvisited, Entered, Exited };

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

  for (const std::string& key : g.ontology_relation() | std::views::keys) {
    std::vector<std::vector<NodeId>> adj;
    adj.resize(g.next_id_node());
    NodeId max = 0;
    g.ForEachLiveRelation([&key, &adj, &max](const Relation& rel) {
      if (rel.type() == key) {
        adj[rel.from()].push_back(rel.to());
        if (rel.from() > max) {
          max = rel.from();
        }
        if (rel.to() > max) {
          max = rel.to();
        }
      }
      return true;
    });

    adj.resize(max + 1);
    const NodeId n = static_cast<NodeId>(adj.size());

    std::vector<VColor> color(n, VColor::Unvisited);
    std::vector<NodeId> parent(n, kNoParent);
    std::vector<Frame> stack;
    stack.reserve(n);

    for (NodeId s = 0; s < n; ++s) {
      if (color[s] != VColor::Unvisited) {
        continue;
      }
      stack.push_back({s, 0, Phase::Enter});

      while (!stack.empty()) {
        Frame& f = stack.back();
        switch (f.phase) {
          case Phase::Enter:
            color[f.v] = VColor::Entered;
            f.phase = Phase::Scan;
            break;

          case Phase::Scan:
            if (f.next < adj[f.v].size()) {
              const NodeId u = adj[f.v][f.next++];
              if (color[u] == VColor::Entered) {
                // Chain of tree edges from u up to f.v, reversed, so the hint
                // lists edges in traversal order with the closing edge last.
                std::vector<Edge> path;
                for (NodeId cur = f.v; cur != u; cur = parent[cur]) {
                  path.push_back({parent[cur], cur});
                }
                std::reverse(path.begin(), path.end());

                // Candidate ids per path edge: parallel relations of the same
                // type resolve into several ids for one (from, to) pair.
                std::vector<std::vector<RelationId>> choices;
                choices.reserve(path.size() + 1);
                for (const Edge& e : path) {
                  choices.push_back(g.ResolveRelationIds(e.from, e.to, key));
                }
                choices.push_back(g.ResolveRelationIds(f.v, u, key));

                // Each combination materializes into its own hint, so the UI
                // renders one firefly per concrete cycle.
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
              if (color[u] == VColor::Unvisited) {
                parent[u] = f.v;
                stack.push_back({u, 0, Phase::Enter});
              }
            } else {
              f.phase = Phase::Exit;
            }
            break;

          case Phase::Exit:
            color[f.v] = VColor::Exited;
            stack.pop_back();
            break;
        }
      }
    }
  }

  return result;
}

}  // namespace ravel

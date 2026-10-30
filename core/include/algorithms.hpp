#ifndef RAVEL_CORE_INCLUDE_ALGORITHMS_HPP_
#define RAVEL_CORE_INCLUDE_ALGORITHMS_HPP_

#include "graph.hpp"

#include <string>
#include <vector>

namespace ravel {

struct CycleHint {
  std::string type;
  std::vector<RelationId> edges;
};

std::vector<CycleHint> FindCycleHints(const Graph& g);

}  // namespace ravel

#endif  // RAVEL_CORE_INCLUDE_ALGORITHMS_HPP_

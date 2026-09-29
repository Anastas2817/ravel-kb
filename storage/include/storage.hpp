#ifndef RAVEL_STORAGE_INCLUDE_STORAGE_HPP_
#define RAVEL_STORAGE_INCLUDE_STORAGE_HPP_

#include "graph.hpp"

#include <string>

namespace ravel {

class JsonStorage {
 public:
  static std::string Save(const Graph& g);
  static Graph Load(const std::string& Json);

 private:
};

}  // namespace ravel

#endif  // RAVEL_STORAGE_INCLUDE_STORAGE_HPP_
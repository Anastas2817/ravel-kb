#ifndef RAVEL_STORAGE_INCLUDE_STORAGE_HPP_
#define RAVEL_STORAGE_INCLUDE_STORAGE_HPP_

#include "graph.hpp"

#include <istream>
#include <ostream>
#include <string>

namespace ravel {

class JsonStorage {
 public:
  static void Save(const GraphSnapshot& g, std::ostream& out);
  static GraphSnapshot Load(std::istream& in);
  static constexpr std::uint64_t kFormatVersion = 2;
};

}  // namespace ravel

#endif  // RAVEL_STORAGE_INCLUDE_STORAGE_HPP_
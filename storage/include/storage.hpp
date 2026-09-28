#pragma once

#include <string>
#include "graph.hpp"

class JsonStorage {
 public:
  static std::string Save(const Graph& g);
  static Graph Load(const std::string& Json);
};
#include "storage.hpp"

std::string JsonStorage::Save(const Graph& g) {
  g.NextIdNode();
  return "";
 }

Graph JsonStorage::Load(const std::string& Json) {
  Graph g;
  std::string temp = Json;
  return g;
}
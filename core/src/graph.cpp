#include "graph.hpp"

NodeId Graph::AddNode(std::string title, std::string type, std::string description, double size, std::optional<std::pair<double, double>> hand_position) {
  return NextIdNode();
}
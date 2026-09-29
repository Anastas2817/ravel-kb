#include "storage.hpp"

namespace ravel {

/*
{
  "format_version": 2,
    "next_id_node" : 4,
    "next_id_relation" : 6,
    "node_types" : [
  {"name": "function", "color" : "#4a90d9", "shape" : "rect",
  "frame_color" : "#2c5f94"},
  { "name": "model", "color" : "#7b68ee", "shape" : "ellipse",
   "frame_color" : "#4b3d8f" }
    ] ,
    "relation_types": [
  {"name": "is_a", "color" : "#4a90d9", "arrow" : "solid",
  "symmetric" : false, "transitive" : true},
  { "name": "is_defined_on", "color" : "#7b68ee", "arrow" : "solid",
   "symmetric" : false, "transitive" : false }
    ] ,
    "nodes": [
  {"id": 1, "title" : "Степенная функция", "type" : "function",
  "description" : "f(x) = x^n"},
  { "id": 2, "title" : "Квадратичная функция", "type" : "function" },
  { "id": 3, "title" : "Вещественная прямая", "type" : "model" }
    ] ,
    "relations": [
  {"id": 4, "from" : 2, "to" : 1, "type" : "is_a"},
  { "id": 5, "from" : 1, "to" : 3, "type" : "is_defined_on",
   "description" : "степенная функция с натуральным показателем задана на всей прямой" }
    ] ,
    "allowed_cycles" : []
}
*/

std::string JsonStorage::Save(const Graph& g) {
  g.NextIdNode();
  return "";
}

Graph JsonStorage::Load(const std::string& Json) {
  Graph g;
  std::string temp = Json;

  // Validate-then-apply `format_version` → счётчики против max id своего вида → словари типов → тип
  // каждого узла/ребра в соответствующем словаре
  return g;
}

}  // namespace ravel
#include <algorithm>

// Анализатор Core Guidelines шумит на заголовках nlohmann (C26495/C26819) —
// та же политика, что и в tests/storage_test.cpp для gtest.
#pragma warning(push)
#pragma warning(disable : 26495 26819)
#include <nlohmann/json.hpp>
#pragma warning(pop)

#include "storage.hpp"

using nlohmann::json;

namespace ravel {

void JsonStorage::Save(const GraphSnapshot& g, std::ostream& out) {
  if (!out) {
    throw std::runtime_error("JsonStorage::Save: output stream is in fail state");
  }
  json j;
  j["format_version"] = 2;
  j["next_id_node"] = g.next_id_node;
  j["next_id_relation"] = g.next_id_relation;

  j["node_types"] = json::array();
  std::vector<std::pair<std::string, NodeType>> node_types(g.ontology_node.begin(),
                                                           g.ontology_node.end());
  std::sort(node_types.begin(), node_types.end(), 
    [](const auto& a, const auto& b) { return a.first < b.first; });
  for (const auto& [name, node_type] : node_types) {
    json type;
    type["name"] = name;
    type["color"] = node_type.Color();
    type["shape"] = node_type.Shape();
    type["frame_color"] = node_type.FrameColor();
    j["node_types"].push_back(type);
  }

  j["relation_types"] = json::array();
  std::vector<std::pair<std::string, RelationType>> relation_types(g.ontology_relation.begin(),
                                                           g.ontology_relation.end());
  std::sort(relation_types.begin(), relation_types.end(), 
    [](const auto& a, const auto& b) { return a.first < b.first; });
  for (const auto& [name, relation_type] : relation_types) {
    json type;
    type["name"] = name;
    type["color"] = relation_type.Color();
    type["arrow"] = relation_type.Arrow();
    type["symmetric"] = relation_type.Symmetric();
    type["transitive"] = relation_type.Transitive();
    j["relation_types"].push_back(type);
  }

  j["nodes"] = json::array();
  for (const auto& node: g.nodes) {
    json obj;
    obj["id"] = node.Id();
    obj["title"] = node.Title();
    obj["type"] = node.Type();
    if (node.Description() != "") {
      obj["description"] = node.Description();
    }
    if (node.Size() != 1) {
      obj["size"] = node.Size();
    }
    if (node.HandPosition() != std::nullopt) {
      obj["hand_position"] =
          json{{"x", node.HandPosition().value().first}, {"y", node.HandPosition().value().second}};
    }
    j["nodes"].push_back(obj);
  }

  j["relations"] = json::array();
  for (const auto& relation: g.relations) {
    json obj;
    obj["id"] = relation.Id();
    obj["from"] = relation.From();
    obj["to"] = relation.To();
    obj["type"] = relation.Type();
    if (relation.Description() != "") {
      obj["description"] = relation.Description();
    }
    j["relations"].push_back(obj);
  }

  j["allowed_cycles"] = json::array();

  out << j.dump(2);
}

GraphSnapshot JsonStorage::Load(std::istream& in) {
  if (!in) {
    throw std::runtime_error("JsonStorage::Load: input stream is in fail state");
  }
  try {
    json j = json::parse(in);
    std::uint64_t format_version = j.at("format_version");
    if (format_version != kFormatVersion) {
      throw std::runtime_error("JsonStorage::Load: wrong format_version");
    }
    GraphSnapshot g;
    g.next_id_node = j.at("next_id_node").get<NodeId>();
    g.next_id_relation = j.at("next_id_relation").get<RelationId>();
    for (const json& el : j.at("node_types")) {
      g.ontology_node.emplace(
          el.at("name").get<std::string>(),
          NodeType(el.at("color").get<std::string>(), el.at("shape").get<std::string>(),
                   el.at("frame_color").get<std::string>()));
    }
    for (const json& el : j.at("relation_types")) {
      g.ontology_relation.emplace(
          el.at("name").get<std::string>(),
          RelationType(el.at("color").get<std::string>(), el.at("arrow").get<std::string>(),
                       el.at("symmetric").get<bool>(), el.at("transitive").get<bool>()));
    }
    for (const json& el : j.at("nodes")) {
      std::optional<std::pair<double, double>> pos = std::nullopt;
      if (el.contains("hand_position")) {
        const json& hp = el.at("hand_position");
        pos = std::make_pair(hp.at("x").get<double>(), hp.at("y").get<double>());
      }
      g.nodes.push_back(Node(el.at("id").get<NodeId>(), el.at("title").get<std::string>(),
                             el.at("type").get<std::string>(), el.value("description", std::string{}),
                             el.value("size", 1.0), pos));
    }
    for (const json& el : j.at("relations")) {
      g.relations.push_back(Relation(el.at("id").get<NodeId>(), el.at("from").get<NodeId>(), 
        el.at("to").get<NodeId>(), el.at("type").get<std::string>(), el.value("description", std::string{})));
    }
    if (j.contains("allowed_cycles") && !j.at("allowed_cycles").is_array()) {
      throw std::runtime_error("JsonStorage::Load: allowed_cycles is not an array");
    }
    return g;
  } catch (const json::exception& e) {
    throw std::runtime_error(std::string("JsonStorage::Load: ") + e.what());
  }
}

}  // namespace ravel
#ifdef _MSC_VER
#pragma warning(disable : 26495 26819)
#endif

#include "storage.hpp"

#include <algorithm>

#include <nlohmann/json.hpp>

using nlohmann::json;

namespace ravel {

void JsonStorage::Save(const GraphSnapshot& g, std::ostream& out) {
  if (!out) {
    throw std::runtime_error("JsonStorage::Save: output stream is in fail state");
  }
  json j;
  j["format_version"] = kFormatVersion;
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
    type["color"] = node_type.color();
    type["shape"] = node_type.shape();
    type["frame_color"] = node_type.frame_color();
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
    type["color"] = relation_type.color();
    type["arrow"] = relation_type.arrow();
    type["symmetric"] = relation_type.symmetric();
    type["transitive"] = relation_type.transitive();
    j["relation_types"].push_back(type);
  }

  j["nodes"] = json::array();
  for (const auto& node : g.nodes) {
    json obj;
    obj["id"] = node.id();
    obj["title"] = node.title();
    obj["type"] = node.type();
    if (node.description() != "") {
      obj["description"] = node.description();
    }
    if (node.size() != 1) {
      obj["size"] = node.size();
    }
    j["nodes"].push_back(obj);
  }

  j["relations"] = json::array();
  for (const auto& relation : g.relations) {
    json obj;
    obj["id"] = relation.id();
    obj["from"] = relation.from();
    obj["to"] = relation.to();
    obj["type"] = relation.type();
    if (relation.description() != "") {
      obj["description"] = relation.description();
    }
    j["relations"].push_back(obj);
  }

  j["positions"] = json::array();
  for (const auto& pos : g.positions) {
    if (pos != std::nullopt) {
      json obj;
      obj["x"] = pos->x_;
      obj["y"] = pos->y_;
      j["positions"].push_back(obj);
    } else {
      j["positions"].push_back(nullptr);
    }
  }

  j["allowed_cycles"] = json::array();  // TODO week 6

  out << j.dump(2);

  if (!out) {
    throw std::runtime_error("JsonStorage::Save: failed to write");
  }
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
      std::string name = el.at("name").get<std::string>();
      if (g.ontology_node.find(name) == g.ontology_node.end()) {
        g.ontology_node.emplace(
            name, NodeType(el.at("color").get<std::string>(), el.at("shape").get<std::string>(),
                           el.at("frame_color").get<std::string>()));
      } else {
        throw std::runtime_error("JsonStorage::Load: repeated NodeTypes");
      }
    }
    for (const json& el : j.at("relation_types")) {
      std::string name = el.at("name").get<std::string>();
      if (g.ontology_relation.find(name) == g.ontology_relation.end()) {
        g.ontology_relation.emplace(
            name, RelationType(el.at("color").get<std::string>(), el.at("arrow").get<std::string>(),
                               el.at("symmetric").get<bool>(), el.at("transitive").get<bool>()));
      } else {
        throw std::runtime_error("JsonStorage::Load: repeated RelationTypes");
      }
    }
    for (const json& el : j.at("nodes")) {
      g.nodes.push_back(Node(el.at("id").get<NodeId>(), el.at("title").get<std::string>(),
                             el.at("type").get<std::string>(),
                             el.value("description", std::string{}), el.value("size", 1.0)));
    }
    for (const json& el : j.at("relations")) {
      g.relations.push_back(Relation(el.at("id").get<RelationId>(), el.at("from").get<NodeId>(),
                                     el.at("to").get<NodeId>(), el.at("type").get<std::string>(),
                                     el.value("description", std::string{})));
    }

    for (const json& el : j.at("positions")) {
      if (!el.is_null()) {
        g.positions.push_back(Position(el.at("x").get<double>(), el.at("y").get<double>()));
      } else {
        g.positions.push_back(std::nullopt);
      }
    }

    if (j.contains("allowed_cycles") && !j.at("allowed_cycles").is_array()) {
      throw std::runtime_error("JsonStorage::Load: allowed_cycles is not an array");
    }
    if (!in) {
      throw std::runtime_error("JsonStorage::Load: stream error");
    }
    return g;
  } catch (const json::exception& e) {
    throw std::runtime_error(std::string("JsonStorage::Load: ") + e.what());
  }
}

}  // namespace ravel

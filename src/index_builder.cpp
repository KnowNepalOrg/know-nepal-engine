#include "engine/index_builder.h"
#include "engine/id_generator.h"
#include "engine/normalize.h"
#include "engine/search_entry.h"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <unordered_set>
#include <nlohmann/json.hpp>

namespace engine {

static void from_json(const nlohmann::json& j, SearchEntry& entry) {
    if (j.contains("id")) j.at("id").get_to(entry.id);
    if (j.contains("name")) j.at("name").get_to(entry.name);
    if (j.contains("nameNepali")) j.at("nameNepali").get_to(entry.nameNepali);
    if (j.contains("module")) j.at("module").get_to(entry.module);
    if (j.contains("type")) j.at("type").get_to(entry.type);
    if (j.contains("href")) j.at("href").get_to(entry.href);
    if (j.contains("aliases")) j.at("aliases").get_to(entry.aliases);
    if (j.contains("keywords")) j.at("keywords").get_to(entry.keywords);
    if (j.contains("location")) j.at("location").get_to(entry.location);
    if (j.contains("popularity")) {
        entry.popularity = j.at("popularity").get<int>();
    }
    if (j.contains("meta")) {
        entry.meta = j.at("meta");
    }
}

static void to_json(nlohmann::json& j, const ProcessedEntry& entry) {
    j["id"] = entry.id;
    j["name"] = entry.name;
    j["nameNepali"] = entry.nameNepali;
    j["module"] = entry.module;
    j["type"] = entry.type;
    j["href"] = entry.href;
    j["aliases"] = entry.aliases;
    j["keywords"] = entry.keywords;
    j["location"] = entry.location;
    if (entry.popularity.has_value()) {
        j["popularity"] = entry.popularity.value();
    }
    j["meta"] = entry.meta;
    j["normalized"] = entry.normalized;
    j["nameTokens"] = entry.nameTokens;
    j["normalizedAliases"] = entry.normalizedAliases;
    j["aliasTokens"] = entry.aliasTokens;
}

static ProcessedEntry process_entry(const SearchEntry& entry) {
    ProcessedEntry pe;
    pe.id = generate_id(entry.module, entry.type, entry.name, entry.id);
    pe.name = entry.name;
    pe.nameNepali = entry.nameNepali;
    pe.module = entry.module;
    pe.type = entry.type;
    pe.href = entry.href;
    pe.aliases = entry.aliases;
    pe.keywords = entry.keywords;
    pe.location = entry.location;
    pe.popularity = entry.popularity;
    pe.meta = entry.meta;
    pe.normalized = normalize(entry.name);
    pe.nameTokens = tokenize(pe.normalized);
    pe.normalizedAliases.resize(entry.aliases.size());
    pe.aliasTokens.resize(entry.aliases.size());
    for (size_t i = 0; i < entry.aliases.size(); ++i) {
        pe.normalizedAliases[i] = normalize(entry.aliases[i]);
        pe.aliasTokens[i] = tokenize(pe.normalizedAliases[i]);
    }
    return pe;
}

void IndexBuilder::process(std::string_view input_path, std::string_view output_path) {
    std::string in_path(input_path);
    std::string out_path(output_path);
    std::ifstream in(in_path);
    if (!in.is_open()) {
        throw std::runtime_error("Cannot open input file: " + in_path);
    }
    nlohmann::json input_json;
    in >> input_json;
    in.close();
    std::vector<SearchEntry> entries;
    if (input_json.is_array()) {
        for (const auto& item : input_json) {
            SearchEntry entry;
            from_json(item, entry);
            entries.push_back(std::move(entry));
        }
    } else {
        throw std::runtime_error("Input JSON must be an array of search entries");
    }
    std::unordered_set<std::string> seen_ids;
    nlohmann::json output_json = nlohmann::json::array();
    for (const auto& entry : entries) {
        ProcessedEntry pe = process_entry(entry);
        if (!seen_ids.insert(pe.id).second) {
            throw std::runtime_error(
                "Duplicate ID generated: \"" + pe.id +
                "\" (from entry name=\"" + entry.name +
                "\", module=\"" + entry.module +
                "\", type=\"" + entry.type + "\")");
        }
        output_json.push_back(std::move(pe));
    }
    std::ofstream out(out_path);
    if (!out.is_open()) {
        throw std::runtime_error("Cannot open output file: " + out_path);
    }
    out << output_json.dump(2) << "\n";
    out.close();
    std::cout << "Processed " << entries.size() << " entries -> " << output_path << "\n";
}

}  // namespace engine

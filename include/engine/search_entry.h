#pragma once

#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <vector>

namespace engine {

struct SearchEntry {
    std::string id;
    std::string name;
    std::string nameNepali;
    std::string module;
    std::string type;
    std::string href;
    std::vector<std::string> aliases;
    std::vector<std::string> keywords;
    std::string location;
    std::optional<int> popularity;
    nlohmann::json meta = nlohmann::json::object();
};

struct ProcessedEntry {
    std::string id;
    std::string name;
    std::string nameNepali;
    std::string module;
    std::string type;
    std::string href;
    std::vector<std::string> aliases;
    std::vector<std::string> keywords;
    std::string location;
    std::optional<int> popularity;
    nlohmann::json meta = nlohmann::json::object();
    std::string normalized;
    std::vector<std::string> nameTokens;
    std::vector<std::string> normalizedAliases;
    std::vector<std::vector<std::string>> aliasTokens;
};

}  // namespace engine

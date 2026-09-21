#include "engine/search.h"
#include "engine/search_entry.h"

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

static void print_usage(const char* prog) {
    std::cerr << "Usage: " << prog << " <index.json> <query> [--limit N]\n";
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        print_usage(argv[0]);
        return 1;
    }

    const char* index_path = argv[1];
    const char* query = argv[2];
    size_t limit = 24;

    for (int i = 3; i < argc; ++i) {
        if (std::strcmp(argv[i], "--limit") == 0 && i + 1 < argc) {
            limit = std::stoul(argv[++i]);
        }
    }

    std::ifstream in(index_path);
    if (!in.is_open()) {
        std::cerr << "Error: Cannot open index file: " << index_path << "\n";
        return 1;
    }

    nlohmann::json index_json;
    in >> index_json;
    in.close();

    std::vector<engine::ProcessedEntry> entries;
    for (const auto& item : index_json) {
        engine::ProcessedEntry entry;
        entry.id = item.value("id", "");
        entry.name = item.value("name", "");
        entry.nameNepali = item.value("nameNepali", "");
        entry.module = item.value("module", "");
        entry.type = item.value("type", "");
        entry.href = item.value("href", "");
        entry.aliases = item.value("aliases", std::vector<std::string>{});
        entry.keywords = item.value("keywords", std::vector<std::string>{});
        entry.location = item.value("location", "");
        if (item.contains("popularity")) {
            entry.popularity = item["popularity"].get<int>();
        }
        entry.normalized = item.value("normalized", "");
        entry.nameTokens = item.value("nameTokens", std::vector<std::string>{});
        entry.normalizedAliases = item.value("normalizedAliases", std::vector<std::string>{});
        entry.aliasTokens = item.value("aliasTokens", std::vector<std::vector<std::string>>{});
        entries.push_back(std::move(entry));
    }

    engine::Searcher searcher(std::move(entries));
    auto results = searcher.search(query, limit);

    std::cout << "Results for \"" << query << "\" (" << results.size() << " hits):\n\n";

    for (size_t i = 0; i < results.size(); ++i) {
        const auto& r = results[i];
        const char* kind = "none";
        switch (r.matchKind) {
            case engine::MatchKind::Exact:     kind = "exact"; break;
            case engine::MatchKind::Alias:     kind = "alias"; break;
            case engine::MatchKind::Substring: kind = "substring"; break;
            case engine::MatchKind::Fuzzy:     kind = "fuzzy"; break;
            case engine::MatchKind::Keyword:   kind = "keyword"; break;
            case engine::MatchKind::None:      kind = "none"; break;
        }
        std::cout << (i + 1) << ". [" << kind << "] score=" << r.score
                  << "  " << r.entry.name
                  << "  (" << r.entry.module << "/" << r.entry.type << ")"
                  << "  " << r.entry.href << "\n";
    }

    return 0;
}

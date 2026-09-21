// C++ benchmark CLI: loads processed JSON, runs search, outputs results + timing.
// Mirrors bench-ts.ts exactly for apples-to-apples comparison.

#include "engine/search.h"
#include "engine/search_entry.h"

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <numeric>
#include <string>
#include <vector>

static void print_usage(const char* prog) {
    std::cerr << "Usage: " << prog
              << " <processed.json> [--iterations N] [--warmup N]\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    const char* index_path = argv[1];
    int iterations = 10000;
    int warmup = 1000;

    for (int i = 2; i < argc; ++i) {
        if (std::strcmp(argv[i], "--iterations") == 0 && i + 1 < argc) {
            iterations = std::atoi(argv[++i]);
        } else if (std::strcmp(argv[i], "--warmup") == 0 && i + 1 < argc) {
            warmup = std::atoi(argv[++i]);
        }
    }

    // Load processed index (excluded from timing)
    std::ifstream in(index_path);
    if (!in.is_open()) {
        std::cerr << "Error: Cannot open index file: " << index_path << "\n";
        return 1;
    }

    nlohmann::json index_json;
    in >> index_json;
    in.close();

    std::vector<engine::ProcessedEntry> entries;
    entries.reserve(index_json.size());
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

    // Query set — must match bench-ts.ts exactly
    std::vector<std::string> queries = {
        "kathmandu", "dashain", "chitwan", "Kathmandu Durbar Square",
        "tiger", "nepal", "kath", "tu", "province 1", "swayambhu",
        "bhaktapur", "lumbini", "annapurna", "everest", "gurung",
        "nepali", "school", "hospital", "university", "festival"
    };

    const size_t limit = 24;

    // Run and output results (excluded from timing)
    engine::Searcher searcher(entries);
    nlohmann::json all_results = nlohmann::json::object();

    for (const auto& query : queries) {
        auto results = searcher.search(query, limit);
        nlohmann::json qr = nlohmann::json::array();
        for (const auto& r : results) {
            const char* kind = "none";
            switch (r.matchKind) {
                case engine::MatchKind::Exact:     kind = "exact"; break;
                case engine::MatchKind::Alias:     kind = "alias"; break;
                case engine::MatchKind::Substring: kind = "substring"; break;
                case engine::MatchKind::Fuzzy:     kind = "fuzzy"; break;
                case engine::MatchKind::Keyword:   kind = "keyword"; break;
                case engine::MatchKind::None:      kind = "none"; break;
            }
            qr.push_back({
                {"name", r.entry.name},
                {"score", r.score},
                {"matchKind", kind},
                {"module", r.entry.module},
                {"type", r.entry.type}
            });
        }
        all_results[query] = std::move(qr);
    }

    std::ofstream out_results("cpp_results.json");
    if (out_results.is_open()) {
        out_results << all_results.dump(2) << "\n";
        out_results.close();
    }

    std::cout << "C++: " << queries.size() << " queries over "
              << entries.size() << " entries\n";

    // Performance measurement (search only, no I/O)
    // Warmup
    for (int i = 0; i < warmup; ++i) {
        for (const auto& query : queries) {
            searcher.search(query, limit);
        }
    }

    // Timed run
    std::vector<double> latencies;
    latencies.reserve(iterations);

    for (int iter = 0; iter < iterations; ++iter) {
        auto start = std::chrono::high_resolution_clock::now();
        for (const auto& query : queries) {
            searcher.search(query, limit);
        }
        auto end = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(end - start).count();
        latencies.push_back(ms);
    }

    std::sort(latencies.begin(), latencies.end());
    double sum = std::accumulate(latencies.begin(), latencies.end(), 0.0);
    double avg = sum / latencies.size();
    double p50 = latencies[static_cast<size_t>(latencies.size() * 0.5)];
    double p95 = latencies[static_cast<size_t>(latencies.size() * 0.95)];
    double p99 = latencies[static_cast<size_t>(latencies.size() * 0.99)];

    nlohmann::json timing = {
        {"engine", "cpp"},
        {"datasetSize", entries.size()},
        {"queryCount", queries.size()},
        {"iterations", iterations},
        {"warmup", warmup},
        {"avgMs", std::round(avg * 10000.0) / 10000.0},
        {"p50Ms", std::round(p50 * 10000.0) / 10000.0},
        {"p95Ms", std::round(p95 * 10000.0) / 10000.0},
        {"p99Ms", std::round(p99 * 10000.0) / 10000.0},
        {"totalMs", std::round(sum * 10000.0) / 10000.0}
    };

    std::ofstream out_timing("cpp_timing.json");
    if (out_timing.is_open()) {
        out_timing << timing.dump(2) << "\n";
        out_timing.close();
    }

    std::cout << "C++ timing: " << timing.dump(2) << "\n";
    return 0;
}

#pragma once

#include "engine/search_entry.h"

#include <string>
#include <string_view>
#include <vector>

namespace engine {

enum class MatchKind {
    Exact,
    Alias,
    Substring,
    Fuzzy,
    Keyword,
    None
};

struct SearchResult {
    ProcessedEntry entry;
    double score;
    MatchKind matchKind;
};

class Searcher {
public:
    explicit Searcher(std::vector<ProcessedEntry> entries);

    // Searches the index for entries matching the query.
    // Returns results sorted by score descending, then name alphabetically.
    // Results with MatchKind::None are excluded.
    std::vector<SearchResult> search(std::string_view query, size_t limit = 24) const;

private:
    std::vector<ProcessedEntry> entries_;

    // Scores a single entry against the query using the TypeScript-compatible cascade.
    SearchResult scoreEntry(const ProcessedEntry& entry, std::string_view query) const;
};

}  // namespace engine

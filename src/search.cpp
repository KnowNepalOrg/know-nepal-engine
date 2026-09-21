#include "engine/search.h"

#include "engine/levenshtein.h"
#include "engine/normalize.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>
#include <vector>

namespace engine {

static constexpr double MIN_SIMILARITY = 0.72;

Searcher::Searcher(std::vector<ProcessedEntry> entries)
    : entries_(std::move(entries)) {}

// Case-insensitive string contains check on normalized (lowercase) strings.
// Both `haystack` and `needle` must already be normalized.
static bool contains_ci(std::string_view haystack, std::string_view needle) {
    if (needle.size() > haystack.size()) return false;
    for (size_t i = 0; i <= haystack.size() - needle.size(); ++i) {
        if (haystack.substr(i, needle.size()) == needle) {
            return true;
        }
    }
    return false;
}

// Case-insensitive starts_with check on normalized strings.
static bool starts_with_ci(std::string_view haystack, std::string_view needle) {
    if (needle.size() > haystack.size()) return false;
    return haystack.substr(0, needle.size()) == needle;
}

// Token-level prefix matching: every query token must prefix-match at least one name token.
// Matches nt.startsWith(qt) || qt.startsWith(nt) in either direction.
static bool token_prefix_match(const std::vector<std::string>& nameTokens,
                               const std::vector<std::string>& queryTokens) {
    for (const auto& qt : queryTokens) {
        bool matched = false;
        for (const auto& nt : nameTokens) {
            if (starts_with_ci(nt, qt) || starts_with_ci(qt, nt)) {
                matched = true;
                break;
            }
        }
        if (!matched) return false;
    }
    return true;
}

SearchResult Searcher::scoreEntry(const ProcessedEntry& entry, std::string_view query) const {
    std::string q = normalize(query);

    if (q.empty()) {
        return {entry, 0.0, MatchKind::None};
    }

    // Priority 1: exact name match
    if (entry.normalized == q) {
        return {entry, 100.0, MatchKind::Exact};
    }

    // Priority 2: exact nameNepali match
    if (!entry.nameNepali.empty()) {
        std::string nepaliNorm = normalize(entry.nameNepali);
        if (nepaliNorm == q) {
            return {entry, 99.0, MatchKind::Exact};
        }
    }

    // Priority 3: exact alias match
    for (const auto& normAlias : entry.normalizedAliases) {
        if (normAlias == q) {
            return {entry, 97.0, MatchKind::Alias};
        }
    }

    // Priority 4: name starts with query
    if (starts_with_ci(entry.normalized, q)) {
        return {entry, 92.0, MatchKind::Substring};
    }

    // Priority 6: alias starts with query
    for (const auto& normAlias : entry.normalizedAliases) {
        if (starts_with_ci(normAlias, q)) {
            return {entry, 88.0, MatchKind::Alias};
        }
    }

    // Priority 5: name contains query
    if (contains_ci(entry.normalized, q)) {
        return {entry, 86.0, MatchKind::Substring};
    }

    // Priority 7: alias contains query
    for (const auto& normAlias : entry.normalizedAliases) {
        if (contains_ci(normAlias, q)) {
            return {entry, 80.0, MatchKind::Alias};
        }
    }

    // Priority 8: token-level prefix match
    auto queryTokens = tokenize(q);
    if (!queryTokens.empty() && token_prefix_match(entry.nameTokens, queryTokens)) {
        double score = 82.0 - static_cast<double>(queryTokens.size()) * 3.0;
        return {entry, score, MatchKind::Fuzzy};
    }

    // Priority 9: Levenshtein similarity on name
    double nameSim = similarity(entry.normalized, q);
    if (nameSim >= MIN_SIMILARITY) {
        return {entry, 60.0 + nameSim * 30.0, MatchKind::Fuzzy};
    }

    // Priority 10: Levenshtein similarity on nameNepali
    if (!entry.nameNepali.empty()) {
        std::string nepaliNorm = normalize(entry.nameNepali);
        double nepaliSim = similarity(nepaliNorm, q);
        if (nepaliSim >= MIN_SIMILARITY) {
            return {entry, 55.0 + nepaliSim * 30.0, MatchKind::Fuzzy};
        }
    }

    // Priority 11: keyword match (bidirectional containment)
    for (const auto& keyword : entry.keywords) {
        std::string kw = normalize(keyword);
        if (contains_ci(q, kw) || contains_ci(kw, q)) {
            return {entry, 40.0, MatchKind::Keyword};
        }
    }

    return {entry, 0.0, MatchKind::None};
}

std::vector<SearchResult> Searcher::search(std::string_view query, size_t limit) const {
    std::vector<SearchResult> results;

    for (const auto& entry : entries_) {
        auto result = scoreEntry(entry, query);
        if (result.matchKind != MatchKind::None) {
            // Popularity boost: up to +8 points
            if (entry.popularity.has_value()) {
                result.score += (entry.popularity.value() / 100.0) * 8.0;
            }
            results.push_back(std::move(result));
        }
    }

    // Sort by score descending, then name alphabetically
    std::sort(results.begin(), results.end(), [](const SearchResult& a, const SearchResult& b) {
        if (a.score != b.score) return a.score > b.score;
        return a.entry.name < b.entry.name;
    });

    if (results.size() > limit) {
        results.resize(limit);
    }

    return results;
}

}  // namespace engine

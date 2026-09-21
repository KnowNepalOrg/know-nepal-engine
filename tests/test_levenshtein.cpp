#include "engine/levenshtein.h"

#include <gtest/gtest.h>
#include <cmath>

using namespace engine;

// --- levenshtein ---

TEST(LevenshteinTest, BothEmpty) {
    EXPECT_EQ(levenshtein("", ""), 0);
}

TEST(LevenshteinTest, OneEmpty) {
    EXPECT_EQ(levenshtein("abc", ""), 3);
    EXPECT_EQ(levenshtein("", "abc"), 3);
}

TEST(LevenshteinTest, Identical) {
    EXPECT_EQ(levenshtein("hello", "hello"), 0);
}

TEST(LevenshteinTest, SingleInsertion) {
    EXPECT_EQ(levenshtein("cat", "cats"), 1);
}

TEST(LevenshteinTest, SingleDeletion) {
    EXPECT_EQ(levenshtein("cats", "cat"), 1);
}

TEST(LevenshteinTest, SingleSubstitution) {
    EXPECT_EQ(levenshtein("cat", "bat"), 1);
}

TEST(LevenshteinTest, ClassicExample) {
    EXPECT_EQ(levenshtein("kitten", "sitting"), 3);
}

TEST(LevenshteinTest, CompletelyDifferent) {
    EXPECT_EQ(levenshtein("abc", "xyz"), 3);
}

TEST(LevenshteinTest, CaseSensitive) {
    EXPECT_EQ(levenshtein("abc", "ABC"), 3);
}

TEST(LevenshteinTest, SingleChar) {
    EXPECT_EQ(levenshtein("a", "a"), 0);
    EXPECT_EQ(levenshtein("a", "b"), 1);
}

// --- UTF-16 behavioral compatibility tests ---
// These verify that Levenshtein operates on UTF-16 code units,
// matching TypeScript string.length and a[i] semantics.

TEST(LevenshteinTest, DevanagariIdentical) {
    // "काश्म" is 4 Devanagari chars, each 3 UTF-8 bytes = 12 bytes
    // In UTF-16: 4 code units
    EXPECT_EQ(levenshtein("\u0915\u093E\u0936\u094D", "\u0915\u093E\u0936\u094D"), 0);
}

TEST(LevenshteinTest, DevanagariSubstitution) {
    // One char different: 1 edit in UTF-16
    EXPECT_EQ(levenshtein("\u0915\u093E\u0936\u094D", "\u0915\u093E\u092E\u094D"), 1);
}

TEST(LevenshteinTest, DevanagariVsAscii) {
    // Completely different scripts: max distance = max(len_a, len_b) in UTF-16
    int d = levenshtein("\u0915\u093E", "ab");
    EXPECT_EQ(d, 2);
}

TEST(LevenshteinTest, MixedAsciiDevanagari) {
    // "hello" + Devanagari vs "hello" + different Devanagari
    int d = levenshtein("hello\u0915\u093E", "hello\u0915\u093F");
    EXPECT_EQ(d, 1);
}

// --- similarity ---

TEST(SimilarityTest, BothEmpty) {
    EXPECT_DOUBLE_EQ(similarity("", ""), 1.0);
}

TEST(SimilarityTest, OneEmpty) {
    EXPECT_DOUBLE_EQ(similarity("abc", ""), 0.0);
}

TEST(SimilarityTest, Identical) {
    EXPECT_DOUBLE_EQ(similarity("hello", "hello"), 1.0);
}

TEST(SimilarityTest, CompletelyDifferent) {
    EXPECT_DOUBLE_EQ(similarity("abc", "xyz"), 0.0);
}

TEST(SimilarityTest, OneEdit) {
    // "cat" (3) vs "cats" (4) — distance=1, maxLen=4, sim = 1 - 1/4 = 0.75
    double sim = similarity("cat", "cats");
    EXPECT_NEAR(sim, 0.75, 1e-9);
}

TEST(SimilarityTest, DevanagariIdentical) {
    EXPECT_DOUBLE_EQ(similarity("\u0915\u093E\u0936\u094D", "\u0915\u093E\u0936\u094D"), 1.0);
}

TEST(SimilarityTest, DevanagariOneSubstitution) {
    // 4 chars, 1 edit -> similarity = 1 - 1/4 = 0.75
    double sim = similarity("\u0915\u093E\u0936\u094D", "\u0915\u093E\u092E\u094D");
    EXPECT_NEAR(sim, 0.75, 1e-9);
}

TEST(SimilarityTest, DevanagariPartialOverlap) {
    // "काश्म" (4 UTF-16 code units) vs "काश्माँडु" (6 UTF-16 code units)
    // Levenshtein distance = 3 (3 insertions)
    // maxLen = 6 -> similarity = 1 - 3/6 = 0.5
    double sim = similarity("\u0915\u093E\u0936\u094D", "\u0915\u093E\u0936\u094D\u093E\u0901\u0921\u0941");
    EXPECT_NEAR(sim, 0.5, 1e-9);
}

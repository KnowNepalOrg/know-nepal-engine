#include "engine/normalize.h"

#include <gtest/gtest.h>

using namespace engine;

// --- normalize ---

TEST(NormalizeTest, EmptyString) {
    EXPECT_EQ(normalize(""), "");
}

TEST(NormalizeTest, LowercaseAscii) {
    EXPECT_EQ(normalize("Hello World"), "hello world");
}

TEST(NormalizeTest, UppercaseAscii) {
    EXPECT_EQ(normalize("HELLO"), "hello");
}

TEST(NormalizeTest, MixedCase) {
    EXPECT_EQ(normalize("Kathmandu Durbar Square"), "kathmandu durbar square");
}

TEST(NormalizeTest, PreservesDigits) {
    EXPECT_EQ(normalize("Province 3"), "province 3");
}

TEST(NormalizeTest, StripsSpecialCharacters) {
    EXPECT_EQ(normalize("Swayambhunath!"), "swayambhunath");
}

TEST(NormalizeTest, ReplacesSpecialCharsWithSpace) {
    EXPECT_EQ(normalize("hello-world"), "hello world");
}

TEST(NormalizeTest, CollapsesWhitespace) {
    EXPECT_EQ(normalize("  hello   world  "), "hello world");
}

TEST(NormalizeTest, LeadingTrailingSpaces) {
    EXPECT_EQ(normalize("  hello  "), "hello");
}

TEST(NormalizeTest, TabsAndNewlines) {
    EXPECT_EQ(normalize("hello\tworld\n"), "hello world");
}

TEST(NormalizeTest, PreservesDevanagari) {
    EXPECT_EQ(normalize("\u0915\u093E\u0936\u094D\u092E\u0940\u0930"),
              "\u0915\u093E\u0936\u094D\u092E\u0940\u0930");
}

TEST(NormalizeTest, StripsNonDevanagariUnicode) {
    EXPECT_EQ(normalize("hello\u00E9"), "hello");
}

TEST(NormalizeTest, NepaliWithSpecialChars) {
    EXPECT_EQ(normalize("\u0915\u093E\u0936\u094D\u092E\u0940\u0930!"),
              "\u0915\u093E\u0936\u094D\u092E\u0940\u0930");
}

TEST(NormalizeTest, OnlySpaces) {
    EXPECT_EQ(normalize("   "), "");
}

TEST(NormalizeTest, SpecialCharsOnly) {
    EXPECT_EQ(normalize("!@#$%"), "");
}

TEST(NormalizeTest, DevanagariWithInternalSpaces) {
    EXPECT_EQ(normalize("\u0915\u093E\u0936\u094D\u092E\u093E\u0901\u0921\u0941 \u0926\u0941\u0930\u094D\u092C\u093E\u0930"),
              "\u0915\u093E\u0936\u094D\u092E\u093E\u0901\u0921\u0941 \u0926\u0941\u0930\u094D\u092C\u093E\u0930");
}

TEST(NormalizeTest, MixedAsciiAndDevanagari) {
    EXPECT_EQ(normalize("Hello \u0915\u093E\u0936\u094D\u092E\u0940\u0930"),
              "hello \u0915\u093E\u0936\u094D\u092E\u0940\u0930");
}

TEST(NormalizeTest, DevanagariWithSpecialCharsAndSpaces) {
    EXPECT_EQ(normalize("  \u0915\u093E\u0936\u094D !@# \u092E\u0940\u0930  "),
              "\u0915\u093E\u0936\u094D \u092E\u0940\u0930");
}

// --- tokenize ---

TEST(TokenizeTest, EmptyString) {
    auto tokens = tokenize("");
    EXPECT_TRUE(tokens.empty());
}

TEST(TokenizeTest, SingleWord) {
    auto tokens = tokenize("hello");
    ASSERT_EQ(tokens.size(), 1u);
    EXPECT_EQ(tokens[0], "hello");
}

TEST(TokenizeTest, MultipleWords) {
    auto tokens = tokenize("kathmandu durbar square");
    ASSERT_EQ(tokens.size(), 3u);
    EXPECT_EQ(tokens[0], "kathmandu");
    EXPECT_EQ(tokens[1], "durbar");
    EXPECT_EQ(tokens[2], "square");
}

TEST(TokenizeTest, LeadingTrailingSpaces) {
    auto tokens = tokenize("  hello world  ");
    ASSERT_EQ(tokens.size(), 2u);
    EXPECT_EQ(tokens[0], "hello");
    EXPECT_EQ(tokens[1], "world");
}

TEST(TokenizeTest, MultipleSpaces) {
    auto tokens = tokenize("hello   world");
    ASSERT_EQ(tokens.size(), 2u);
}

TEST(TokenizeTest, DevanagariTokens) {
    auto tokens = tokenize("\u0915\u093E\u0936\u094D\u092E\u093E\u0901\u0921\u0941 \u0926\u0941\u0930\u094D\u092C\u093E\u0930");
    ASSERT_EQ(tokens.size(), 2u);
    EXPECT_EQ(tokens[0], "\u0915\u093E\u0936\u094D\u092E\u093E\u0901\u0921\u0941");
    EXPECT_EQ(tokens[1], "\u0926\u0941\u0930\u094D\u092C\u093E\u0930");
}

// --- slugify ---

TEST(SlugifyTest, EmptyString) {
    EXPECT_EQ(slugify(""), "");
}

TEST(SlugifyTest, SimpleName) {
    EXPECT_EQ(slugify("Kathmandu"), "kathmandu");
}

TEST(SlugifyTest, SpacesToHyphens) {
    EXPECT_EQ(slugify("Durbar Square"), "durbar-square");
}

TEST(SlugifyTest, SpecialCharsToHyphens) {
    EXPECT_EQ(slugify("Swayambhunath!"), "swayambhunath");
}

TEST(SlugifyTest, MultipleSpecialChars) {
    EXPECT_EQ(slugify("hello---world"), "hello-world");
}

TEST(SlugifyTest, LeadingTrailingHyphens) {
    EXPECT_EQ(slugify("  hello  "), "hello");
}

TEST(SlugifyTest, UppercaseToLower) {
    EXPECT_EQ(slugify("EVEREST BASE CAMP"), "everest-base-camp");
}

TEST(SlugifyTest, PreservesDigits) {
    EXPECT_EQ(slugify("Province 3"), "province-3");
}

TEST(SlugifyTest, DevanagariReplacedWithHyphens) {
    EXPECT_EQ(slugify("\u0915\u093E\u0936\u094D"), "");
}

TEST(SlugifyTest, MixedAsciiAndDevanagari) {
    EXPECT_EQ(slugify("Hello \u0915\u093E"), "hello");
}

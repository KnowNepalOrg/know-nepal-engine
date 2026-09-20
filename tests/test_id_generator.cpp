#include "engine/id_generator.h"

#include <gtest/gtest.h>

using namespace engine;

TEST(GenerateIdTest, UsesExistingId) {
    EXPECT_EQ(generate_id("geography", "province", "Bagmati", "my-custom-id"),
              "my-custom-id");
}

TEST(GenerateIdTest, GeneratesFromName) {
    EXPECT_EQ(generate_id("geography", "province", "Bagmati", ""),
              "geography-province-bagmati");
}

TEST(GenerateIdTest, SlugifiesName) {
    EXPECT_EQ(generate_id("destinations", "destination", "Kathmandu Durbar Square", ""),
              "destinations-destination-kathmandu-durbar-square");
}

TEST(GenerateIdTest, HandlesSpecialChars) {
    EXPECT_EQ(generate_id("culture", "festival", "Maha Shivaratri!", ""),
              "culture-festival-maha-shivaratri");
}

TEST(GenerateIdTest, HandlesMixedCase) {
    EXPECT_EQ(generate_id("wildlife", "national-park", "CHITWAN", ""),
              "wildlife-national-park-chitwan");
}

TEST(GenerateIdTest, EmptyName) {
    EXPECT_EQ(generate_id("education", "school", "", ""),
              "education-school-");
}

TEST(GenerateIdTest, PreservesExistingIdWithSlug) {
    EXPECT_EQ(generate_id("culture", "festival", "Dashain", "custom-dashain"),
              "custom-dashain");
}

TEST(GenerateIdTest, DevanagariName) {
    EXPECT_EQ(generate_id("geography", "province", "\u0915\u093E\u0936\u094D", ""),
              "geography-province-");
}

#include "engine/index_builder.h"

#include <cstdio>
#include <fstream>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

using namespace engine;

static std::string temp_path(const char* name) {
    std::string p = std::tmpnam(nullptr);
    p += name;
    return p;
}

static nlohmann::json read_json(const std::string& path) {
    std::ifstream f(path);
    nlohmann::json j;
    f >> j;
    return j;
}

class IndexBuilderTest : public ::testing::Test {
protected:
    std::string input_path;
    std::string output_path;

    void SetUp() override {
        input_path = temp_path("_input.json");
        output_path = temp_path("_output.json");
    }

    void TearDown() override {
        std::remove(input_path.c_str());
        std::remove(output_path.c_str());
    }

    void write_input(const nlohmann::json& j) {
        std::ofstream f(input_path);
        f << j.dump();
    }

    nlohmann::json run_builder() {
        IndexBuilder builder;
        builder.process(input_path, output_path);
        return read_json(output_path);
    }
};

TEST_F(IndexBuilderTest, PreservesMeta) {
    write_input(nlohmann::json::array({
        {
            {"name", "Kathmandu"},
            {"module", "geography"},
            {"type", "municipality"},
            {"href", "/geography/municipalities/kathmandu"},
            {"meta", {{"districtId", 26}, {"provinceId", 3}}}
        }
    }));
    auto output = run_builder();
    ASSERT_EQ(output.size(), 1u);
    EXPECT_EQ(output[0]["meta"]["districtId"], 26);
    EXPECT_EQ(output[0]["meta"]["provinceId"], 3);
}

TEST_F(IndexBuilderTest, PreservesEmptyMeta) {
    write_input(nlohmann::json::array({
        {
            {"name", "Test"},
            {"module", "culture"},
            {"type", "festival"},
            {"href", "/culture/festivals/test"},
            {"meta", nlohmann::json::object()}
        }
    }));
    auto output = run_builder();
    ASSERT_EQ(output.size(), 1u);
    EXPECT_TRUE(output[0]["meta"].is_object());
    EXPECT_TRUE(output[0]["meta"].empty());
}

TEST_F(IndexBuilderTest, PreservesEmptyAliases) {
    write_input(nlohmann::json::array({
        {
            {"name", "Test Hospital"},
            {"module", "healthcare"},
            {"type", "hospital"},
            {"href", "/healthcare/hospitals/test"},
            {"aliases", nlohmann::json::array()},
            {"keywords", nlohmann::json::array()}
        }
    }));
    auto output = run_builder();
    ASSERT_EQ(output.size(), 1u);
    EXPECT_TRUE(output[0]["aliases"].is_array());
    EXPECT_TRUE(output[0]["aliases"].empty());
    EXPECT_TRUE(output[0]["keywords"].is_array());
    EXPECT_TRUE(output[0]["keywords"].empty());
}

TEST_F(IndexBuilderTest, PreservesNameNepali) {
    write_input(nlohmann::json::array({
        {
            {"name", "Kathmandu"},
            {"nameNepali", "\u0915\u093E\u0936\u093E"},
            {"module", "geography"},
            {"type", "municipality"},
            {"href", "/geography/municipalities/kathmandu"}
        }
    }));
    auto output = run_builder();
    ASSERT_EQ(output.size(), 1u);
    EXPECT_EQ(output[0]["nameNepali"], "\u0915\u093E\u0936\u093E");
}

TEST_F(IndexBuilderTest, PreservesAllInputFields) {
    write_input(nlohmann::json::array({
        {
            {"id", "custom-id"},
            {"name", "Test"},
            {"nameNepali", "\u0925\u0947\u0938\u094D\u091F"},
            {"module", "geography"},
            {"type", "province"},
            {"href", "/geography/provinces/test"},
            {"aliases", {"Alt Name"}},
            {"keywords", {"keyword1"}},
            {"location", "Test Location"},
            {"popularity", 42},
            {"meta", {{"key", "value"}}}
        }
    }));
    auto output = run_builder();
    ASSERT_EQ(output.size(), 1u);
    EXPECT_EQ(output[0]["id"], "custom-id");
    EXPECT_EQ(output[0]["name"], "Test");
    EXPECT_EQ(output[0]["nameNepali"], "\u0925\u0947\u0938\u094D\u091F");
    EXPECT_EQ(output[0]["module"], "geography");
    EXPECT_EQ(output[0]["type"], "province");
    EXPECT_EQ(output[0]["href"], "/geography/provinces/test");
    EXPECT_EQ(output[0]["aliases"], nlohmann::json::array({"Alt Name"}));
    EXPECT_EQ(output[0]["keywords"], nlohmann::json::array({"keyword1"}));
    EXPECT_EQ(output[0]["location"], "Test Location");
    EXPECT_EQ(output[0]["popularity"], 42);
    EXPECT_EQ(output[0]["meta"]["key"], "value");
}

TEST_F(IndexBuilderTest, AddsGeneratedFields) {
    write_input(nlohmann::json::array({
        {
            {"name", "Kathmandu"},
            {"module", "geography"},
            {"type", "municipality"},
            {"href", "/geography/municipalities/kathmandu"},
            {"aliases", {"Kathmandu City"}}
        }
    }));
    auto output = run_builder();
    ASSERT_EQ(output.size(), 1u);
    EXPECT_EQ(output[0]["normalized"], "kathmandu");
    EXPECT_EQ(output[0]["nameTokens"], nlohmann::json::array({"kathmandu"}));
    EXPECT_EQ(output[0]["normalizedAliases"], nlohmann::json::array({"kathmandu city"}));
    EXPECT_EQ(output[0]["aliasTokens"], nlohmann::json::array({{"kathmandu", "city"}}));
}

TEST_F(IndexBuilderTest, DuplicateIDDetection) {
    write_input(nlohmann::json::array({
        {
            {"name", "Same Name"},
            {"module", "geography"},
            {"type", "province"},
            {"href", "/a"}
        },
        {
            {"name", "Same Name"},
            {"module", "geography"},
            {"type", "province"},
            {"href", "/b"}
        }
    }));
    IndexBuilder builder;
    EXPECT_THROW(builder.process(input_path, output_path), std::runtime_error);
}

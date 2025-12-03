#include "../fixtures/TestFixture.hpp"
#include "Utils.h"
#include <nlohmann/json.hpp>
#include <fstream>

class UtilsTest : public TestFixture
{
protected:
    void SetUp() override
    {
        TestFixture::SetUp();
        create_test_json_file();
    }

    void TearDown() override
    {
        cleanup_test_files();
        TestFixture::TearDown();
    }

    void create_test_json_file()
    {
        nlohmann::json test_data = {
            {"test_key", "test_value"},
            {"number", 42}};

        std::ofstream file("test_utils.json");
        file << test_data.dump(4);
        file.close();
    }

    void cleanup_test_files()
    {
        std::remove("test_utils.json");
    }
};

TEST_F(UtilsTest, ReadJsonFunction)
{
    nlohmann::json result = ReadJson("test_utils.json");
    EXPECT_TRUE(result.contains("test_key"));
    EXPECT_EQ(result["test_key"], "test_value");
    EXPECT_EQ(result["number"], 42);
}

TEST_F(UtilsTest, ReadDataFileFunction)
{
    DataFile result = ReadDataFile("test_utils.json");
    EXPECT_TRUE(result.contains("test_key"));
    EXPECT_EQ(result["test_key"], "test_value");
}

TEST_F(UtilsTest, ReadJsonInvalidFile)
{
    nlohmann::json result = ReadJson("nonexistent_file.json");
    // Should return empty JSON object for invalid files
    EXPECT_TRUE(result.empty());
}

TEST_F(UtilsTest, ReadDataFileInvalidFile)
{
    DataFile result = ReadDataFile("nonexistent_file.json");
    // Should handle invalid files gracefully
    EXPECT_TRUE(result.empty());
}

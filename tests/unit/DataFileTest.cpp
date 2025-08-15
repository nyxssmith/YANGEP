#include "../fixtures/TestFixture.hpp"
#include "../../src/lib/DataFile.h"
#include <fstream>
#include <nlohmann/json.hpp>

class DataFileTest : public TestFixture {
protected:
    void SetUp() override {
        TestFixture::SetUp();
        create_test_json_file();
    }

    void TearDown() override {
        cleanup_test_files();
        TestFixture::TearDown();
    }

    void create_test_json_file() {
        nlohmann::json test_data = {
            {"test_key", "test_value"},
            {"number", 42},
            {"array", {1, 2, 3}}
        };

        std::ofstream file("test_data.json");
        file << test_data.dump(4);
        file.close();
    }

    void cleanup_test_files() {
        std::remove("test_data.json");
        std::remove("test_save.json");
    }
};

TEST_F(DataFileTest, ConstructorWithFilename) {
    DataFile df("test_data.json");
    EXPECT_TRUE(df.contains("test_key"));
    EXPECT_EQ(df["test_key"], "test_value");
    EXPECT_EQ(df["number"], 42);
}

TEST_F(DataFileTest, LoadMethod) {
    DataFile df;
    EXPECT_TRUE(df.load("test_data.json"));
    EXPECT_TRUE(df.contains("test_key"));
}

TEST_F(DataFileTest, SaveMethod) {
    DataFile df;
    df["new_key"] = "new_value";

    EXPECT_TRUE(df.save("test_save.json"));

    // Verify saved data
    DataFile loaded_df("test_save.json");
    EXPECT_TRUE(loaded_df.contains("new_key"));
    EXPECT_EQ(loaded_df["new_key"], "new_value");
}

TEST_F(DataFileTest, InvalidFileHandling) {
    DataFile df;
    EXPECT_FALSE(df.load("nonexistent_file.json"));
}

TEST_F(DataFileTest, DefaultConstructor) {
    DataFile df;
    EXPECT_TRUE(df.empty());
}

TEST_F(DataFileTest, SetAndGetFilename) {
    DataFile df;
    df.setFilename("test_filename.json");
    EXPECT_EQ(df.getFilename(), "test_filename.json");
}

TEST_F(DataFileTest, LoadFromStoredFilename) {
    DataFile df;
    df.setFilename("test_data.json");
    EXPECT_TRUE(df.load());
    EXPECT_TRUE(df.contains("test_key"));
}

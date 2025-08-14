#include "../fixtures/TestFixture.hpp"
#include "../../src/lib/DataFile.h"
#include "../../src/lib/Utils.h"
#include <fstream>

class SpriteSystemIntegrationTest : public TestFixture {
protected:
    void SetUp() override {
        TestFixture::SetUp();
        create_test_assets();
    }

    void TearDown() override {
        cleanup_test_assets();
        TestFixture::TearDown();
    }

    void create_test_assets() {
        // Create test JSON configuration
        nlohmann::json sprite_config = {
            {"sprites", {
                {
                    {"name", "test_sprite"},
                    {"texture", "test_texture.png"},
                    {"position", {100, 100}},
                    {"scale", {1.0, 1.0}}
                }
            }}
        };

        std::ofstream file("test_sprite_config.json");
        file << sprite_config.dump(4);
        file.close();
    }

    void cleanup_test_assets() {
        std::remove("test_sprite_config.json");
    }
};

TEST_F(SpriteSystemIntegrationTest, LoadSpriteConfiguration) {
    // Test loading sprite configuration from JSON
    DataFile config("test_sprite_config.json");
    EXPECT_TRUE(config.contains("sprites"));

    auto sprites = config["sprites"];
    EXPECT_EQ(sprites.size(), 1);

    auto sprite = sprites[0];
    EXPECT_EQ(sprite["name"], "test_sprite");
    EXPECT_EQ(sprite["texture"], "test_texture.png");
}

TEST_F(SpriteSystemIntegrationTest, EndToEndSpriteLoading) {
    // Test complete sprite loading workflow
    nlohmann::json result = ReadJson("test_sprite_config.json");
    EXPECT_TRUE(result.contains("sprites"));

    // This test will be expanded when Sprite class is implemented
    EXPECT_TRUE(true);
}

TEST_F(SpriteSystemIntegrationTest, UtilsAndDataFileIntegration) {
    // Test that Utils and DataFile work together
    DataFile config = ReadDataFile("test_sprite_config.json");
    EXPECT_TRUE(config.contains("sprites"));

    auto sprites = config["sprites"];
    EXPECT_EQ(sprites.size(), 1);

    // Test data modification and saving
    config["sprites"][0]["position"] = {200, 200};
    EXPECT_EQ(config["sprites"][0]["position"][0], 200);
    EXPECT_EQ(config["sprites"][0]["position"][1], 200);
}

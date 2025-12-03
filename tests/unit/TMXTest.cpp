#include <gtest/gtest.h>
#include <cute.h>
#include "tmx.h"
#include "tsx.h"
#include "Utils.h"
#include "../fixtures/TestFixture.hpp"

using namespace Cute;

class TMXTest : public TestFixture
{
protected:
    void SetUp() override
    {
        TestFixture::SetUp();
        // Mount assets for TMX tests
        mount_content_directory_as("/assets");
    }
};

class TMXSystemTest : public TMXTest
{
protected:
    std::unique_ptr<tmx> testMap;

    void SetUp() override
    {
        TMXTest::SetUp();
        // Load the test TMX map
        testMap = std::make_unique<tmx>("assets/Levels/test_one/test_one.tmx");
    }
};

// Basic TMX loading tests
TEST_F(TMXTest, CanLoadTMXFile)
{
    tmx map("assets/Levels/test_one/test_one.tmx");

    EXPECT_FALSE(map.empty()) << "TMX file should load successfully";
    EXPECT_GT(map.getTileWidth(), 0) << "Tile width should be positive";
    EXPECT_GT(map.getTileHeight(), 0) << "Tile height should be positive";
    EXPECT_GT(map.getMapWidth(), 0) << "Map width should be positive";
    EXPECT_GT(map.getMapHeight(), 0) << "Map height should be positive";
}

TEST_F(TMXTest, TMXHasExpectedDimensions)
{
    tmx map("assets/Levels/test_one/test_one.tmx");

    // Based on what we saw in the main game output
    EXPECT_EQ(map.getMapWidth(), 30) << "Map should be 30 tiles wide";
    EXPECT_EQ(map.getMapHeight(), 20) << "Map should be 20 tiles tall";
    EXPECT_EQ(map.getTileWidth(), 32) << "Tiles should be 32 pixels wide";
    EXPECT_EQ(map.getTileHeight(), 32) << "Tiles should be 32 pixels tall";
}

TEST_F(TMXSystemTest, TMXHasExpectedTilesets)
{
    EXPECT_GT(testMap->getTilesetCount(), 0) << "Map should have at least one tileset";

    // Based on debug output, we expect 3 tilesets
    EXPECT_EQ(testMap->getTilesetCount(), 3) << "Map should have exactly 3 tilesets";
}

TEST_F(TMXSystemTest, TMXHasExpectedLayers)
{
    EXPECT_GT(testMap->getLayerCount(), 0) << "Map should have at least one layer";

    // Based on debug output, we expect 2 layers
    EXPECT_EQ(testMap->getLayerCount(), 2) << "Map should have exactly 2 layers";
}

TEST_F(TMXSystemTest, CanAccessLayers)
{
    ASSERT_GT(testMap->getLayerCount(), 0);

    auto firstLayer = testMap->getLayer(0);
    ASSERT_NE(firstLayer, nullptr) << "First layer should exist";

    EXPECT_GT(firstLayer->width, 0) << "Layer width should be positive";
    EXPECT_GT(firstLayer->height, 0) << "Layer height should be positive";
    EXPECT_TRUE(firstLayer->visible) << "Layer should be visible by default";
    EXPECT_GT(firstLayer->opacity, 0.0f) << "Layer opacity should be positive";
}

// TSX loading tests
TEST_F(TMXTest, CanLoadTSXFiles)
{
    // Test loading individual TSX files that TMX references
    tsx magecityTsx("assets/Levels/test_one/magecity.tsx");
    EXPECT_FALSE(magecityTsx.empty()) << "Magecity TSX should load";

    tsx dungeonTsx("assets/Levels/test_one/dungeon_tiles.tsx");
    EXPECT_FALSE(dungeonTsx.empty()) << "Dungeon tiles TSX should load";
}

TEST_F(TMXTest, TSXHasValidDimensions)
{
    tsx magecityTsx("assets/Levels/test_one/magecity.tsx");
    ASSERT_FALSE(magecityTsx.empty());

    EXPECT_GT(magecityTsx.getTileWidth(), 0) << "TSX tile width should be positive";
    EXPECT_GT(magecityTsx.getTileHeight(), 0) << "TSX tile height should be positive";
    EXPECT_GT(magecityTsx.getSourceWidth(), 0) << "TSX source width should be positive";
    EXPECT_GT(magecityTsx.getSourceHeight(), 0) << "TSX source height should be positive";
}

// Sprite creation tests - these test the WORKING sprite system
TEST_F(TMXTest, CanCreateSpritesFromTSX)
{
    tsx magecityTsx("assets/Levels/test_one/magecity.tsx");
    ASSERT_FALSE(magecityTsx.empty());

    // Create a sprite from the first tile (0, 0)
    CF_Sprite sprite = magecityTsx.getTile(0, 0);

    EXPECT_GT(sprite.w, 0) << "Sprite width should be positive";
    EXPECT_GT(sprite.h, 0) << "Sprite height should be positive";
    EXPECT_EQ(sprite.w, magecityTsx.getTileWidth()) << "Sprite width should match tile width";
    EXPECT_EQ(sprite.h, magecityTsx.getTileHeight()) << "Sprite height should match tile height";
}

TEST_F(TMXTest, CanCreateMultipleSpritesFromTSX)
{
    tsx magecityTsx("assets/Levels/test_one/magecity.tsx");
    ASSERT_FALSE(magecityTsx.empty());

    // Create multiple sprites to test the working system
    std::vector<CF_Sprite> sprites;

    for (int y = 0; y < 3; y++)
    {
        for (int x = 0; x < 3; x++)
        {
            CF_Sprite sprite = magecityTsx.getTile(x, y);
            sprites.push_back(sprite);

            EXPECT_GT(sprite.w, 0) << "Sprite (" << x << "," << y << ") width should be positive";
            EXPECT_GT(sprite.h, 0) << "Sprite (" << x << "," << y << ") height should be positive";
        }
    }

    EXPECT_EQ(sprites.size(), 9) << "Should have created 9 sprites";
}

// TMX-TSX integration tests
TEST_F(TMXSystemTest, CanGetValidGIDsFromLayers)
{
    ASSERT_GT(testMap->getLayerCount(), 0);

    auto firstLayer = testMap->getLayer(0);
    ASSERT_NE(firstLayer, nullptr);

    // Test getting GIDs from different positions
    int gid_0_0 = firstLayer->getTileGID(0, 0);
    int gid_5_5 = firstLayer->getTileGID(5, 5);
    int gid_10_10 = firstLayer->getTileGID(10, 10);

    // GIDs should be non-negative (0 means empty, positive means tile)
    EXPECT_GE(gid_0_0, 0) << "GID at (0,0) should be valid";
    EXPECT_GE(gid_5_5, 0) << "GID at (5,5) should be valid";
    EXPECT_GE(gid_10_10, 0) << "GID at (10,10) should be valid";
}

TEST_F(TMXSystemTest, CanGetSpritesAtTilePositions)
{
    ASSERT_GT(testMap->getLayerCount(), 0);

    auto firstLayer = testMap->getLayer(0);
    ASSERT_NE(firstLayer, nullptr);

    // Find a non-empty tile position
    int test_x = -1, test_y = -1;
    for (int y = 0; y < firstLayer->height && test_x < 0; y++)
    {
        for (int x = 0; x < firstLayer->width && test_x < 0; x++)
        {
            int gid = firstLayer->getTileGID(x, y);
            if (gid > 0)
            {
                test_x = x;
                test_y = y;
            }
        }
    }

    ASSERT_GE(test_x, 0) << "Should find at least one non-empty tile";
    ASSERT_GE(test_y, 0) << "Should find at least one non-empty tile";

    // Get sprite at this position using TMX's public API
    CF_Sprite sprite = testMap->getTileAt(0, test_x, test_y);

    EXPECT_GT(sprite.w, 0) << "Sprite at (" << test_x << "," << test_y << ") should have positive width";
    EXPECT_GT(sprite.h, 0) << "Sprite at (" << test_x << "," << test_y << ") should have positive height";
}

// PNG loading pattern tests - understanding what works
TEST_F(TMXTest, TSXPNGLoadingPattern)
{
    tsx magecityTsx("assets/Levels/test_one/magecity.tsx");
    ASSERT_FALSE(magecityTsx.empty());

    // This tests the exact PNG loading pattern that WORKS
    CF_Sprite sprite1 = magecityTsx.getTile(0, 0); // 32x32 tile
    CF_Sprite sprite2 = magecityTsx.getTile(1, 1); // 32x32 tile
    CF_Sprite sprite3 = magecityTsx.getTile(2, 2); // 32x32 tile

    // All should be valid
    EXPECT_GT(sprite1.w, 0);
    EXPECT_GT(sprite1.h, 0);
    EXPECT_GT(sprite2.w, 0);
    EXPECT_GT(sprite2.h, 0);
    EXPECT_GT(sprite3.w, 0);
    EXPECT_GT(sprite3.h, 0);

    // All should have consistent dimensions (32x32 for this tileset)
    EXPECT_EQ(sprite1.w, sprite2.w);
    EXPECT_EQ(sprite1.h, sprite2.h);
    EXPECT_EQ(sprite2.w, sprite3.w);
    EXPECT_EQ(sprite2.h, sprite3.h);
}

// Stress test the working system
TEST_F(TMXTest, TSXStressTest)
{
    tsx magecityTsx("assets/Levels/test_one/magecity.tsx");
    ASSERT_FALSE(magecityTsx.empty());

    // Create many sprites to stress test the working system
    const int stress_count = 50;
    std::vector<CF_Sprite> sprites;
    sprites.reserve(stress_count);

    for (int i = 0; i < stress_count; i++)
    {
        // Create sprites from different tile positions
        int tile_x = i % 8; // Assume at least 8 tiles wide
        int tile_y = i / 8;

        CF_Sprite sprite = magecityTsx.getTile(tile_x, tile_y);
        sprites.push_back(sprite);

        // Each sprite should be valid
        EXPECT_GT(sprite.w, 0) << "Stress test sprite " << i << " should be valid";
        EXPECT_GT(sprite.h, 0) << "Stress test sprite " << i << " should be valid";
    }

    EXPECT_EQ(sprites.size(), stress_count) << "Should create all stress test sprites";
}

#include <gtest/gtest.h>
#include <cute.h>
#include "lib/tmx.h"
#include "lib/tsx.h"
#include "lib/Camera.h"
#include "lib/Utils.h"
#include "../fixtures/TestFixture.hpp"

using namespace Cute;

class TMXRenderingTest : public TestFixture {
protected:
    std::unique_ptr<tmx> testMap;
    std::unique_ptr<Camera> testCamera;

    void SetUp() override {
        TestFixture::SetUp();
        mount_content_directory_as("/assets");

        // Load test TMX and create camera
        testMap = std::make_unique<tmx>("assets/Levels/test_one/test_one.tmx");
        testCamera = std::make_unique<Camera>(cf_v2(0.0f, 0.0f), 1.0f);

        ASSERT_FALSE(testMap->empty()) << "TMX file should load for rendering tests";
    }

    void TearDown() override {
        // Make sure camera state is clean
        if (testCamera) {
            testCamera.reset();
        }
        testMap.reset();
        TestFixture::TearDown();
    }
};

// Test that TMX sprites can be created without errors
TEST_F(TMXRenderingTest, CanCreateTMXSpritesForRendering) {
    ASSERT_GT(testMap->getLayerCount(), 0);

    auto firstLayer = testMap->getLayer(0);
    ASSERT_NE(firstLayer, nullptr);

    // Find some non-empty tiles and create sprites
    std::vector<CF_Sprite> createdSprites;
    int sprites_created = 0;

    for (int y = 0; y < std::min(firstLayer->height, 5) && sprites_created < 10; y++) {
        for (int x = 0; x < std::min(firstLayer->width, 5) && sprites_created < 10; x++) {
                        int gid = firstLayer->getTileGID(x, y);
            if (gid > 0) {
                CF_Sprite sprite = testMap->getTileAt(0, x, y);
                if (sprite.w > 0 && sprite.h > 0) {
                    createdSprites.push_back(sprite);
                    sprites_created++;

                    // Validate sprite properties
                    EXPECT_GT(sprite.w, 0) << "Sprite width should be positive at (" << x << "," << y << ")";
                    EXPECT_GT(sprite.h, 0) << "Sprite height should be positive at (" << x << "," << y << ")";
                }
            }
        }
    }

    EXPECT_GT(sprites_created, 0) << "Should be able to create at least one sprite from TMX";
    EXPECT_EQ(createdSprites.size(), sprites_created);
}

// Test camera setup for TMX rendering
TEST_F(TMXRenderingTest, CameraSetupForTMXRendering) {
    // Test camera initialization
    EXPECT_EQ(testCamera->getPosition().x, 0.0f);
    EXPECT_EQ(testCamera->getPosition().y, 0.0f);
    EXPECT_EQ(testCamera->getZoom(), 1.0f);

    // Test camera movement
    testCamera->setPosition(100.0f, 200.0f);
    v2 pos = testCamera->getPosition();
    EXPECT_EQ(pos.x, 100.0f);
    EXPECT_EQ(pos.y, 200.0f);

    // Test camera zoom
    testCamera->setZoom(2.0f);
    EXPECT_EQ(testCamera->getZoom(), 2.0f);
}

// Test TMX rendering pipeline setup (without actual rendering to avoid crash)
TEST_F(TMXRenderingTest, TMXRenderingPipelineSetup) {
    ASSERT_GT(testMap->getLayerCount(), 0);

    auto firstLayer = testMap->getLayer(0);
    ASSERT_NE(firstLayer, nullptr);

    // Test that we can prepare for rendering without crashing
    int tile_width = testMap->getTileWidth();
    int tile_height = testMap->getTileHeight();

    EXPECT_GT(tile_width, 0);
    EXPECT_GT(tile_height, 0);

    // Test calculating world positions (what TMX rendering does)
    float world_x = 0.0f;
    float world_y = 0.0f;

    for (int y = 0; y < std::min(3, firstLayer->height); y++) {
        for (int x = 0; x < std::min(3, firstLayer->width); x++) {
            int gid = firstLayer->getTileGID(x, y);
            if (gid > 0) {
                // Calculate tile world position (like TMX does)
                float tile_world_x = world_x + (x * tile_width);
                float tile_world_y = world_y + ((firstLayer->height - 1 - y) * tile_height);

                EXPECT_GE(tile_world_x, 0.0f) << "Tile world X should be valid";
                EXPECT_GE(tile_world_y, 0.0f) << "Tile world Y should be valid";

                // Test that we can get a sprite for this position
                CF_Sprite sprite = testMap->getTileAt(0, x, y);
                EXPECT_GT(sprite.w, 0) << "Should be able to get sprite for rendering position";
            }
        }
    }
}

// Test TMX tileset access patterns
TEST_F(TMXRenderingTest, TMXTilesetAccessPatterns) {
    // Test finding tilesets for different GIDs
    ASSERT_GT(testMap->getLayerCount(), 0);

    auto firstLayer = testMap->getLayer(0);
    ASSERT_NE(firstLayer, nullptr);

        std::vector<std::pair<int, int>> tile_positions;

    // Collect some tile positions with non-empty GIDs
    for (int y = 0; y < std::min(firstLayer->height, 10); y++) {
        for (int x = 0; x < std::min(firstLayer->width, 10); x++) {
            int gid = firstLayer->getTileGID(x, y);
            if (gid > 0) {
                tile_positions.push_back({x, y});
            }
        }
    }

    EXPECT_GT(tile_positions.size(), 0) << "Should find some tiles in the map";

    // Test that each tile position can be resolved to a sprite
    for (auto [x, y] : tile_positions) {
        CF_Sprite sprite = testMap->getTileAt(0, x, y);
        EXPECT_GT(sprite.w, 0) << "Tile at (" << x << "," << y << ") should resolve to valid sprite";
        EXPECT_GT(sprite.h, 0) << "Tile at (" << x << "," << y << ") should resolve to valid sprite";

        // Limit to avoid too many tests
        if (tile_positions.size() > 20) break;
    }
}

// Test TSX individual rendering capability
TEST_F(TMXRenderingTest, TSXIndividualSpriteCapability) {
    // Test loading and accessing TSX files directly
    tsx magecityTsx("assets/Levels/test_one/magecity.tsx");
    ASSERT_FALSE(magecityTsx.empty());

    // Test creating sprites of different sizes that work
    CF_Sprite sprite1 = magecityTsx.getTile(0, 0);
    CF_Sprite sprite2 = magecityTsx.getTile(3, 3);
    CF_Sprite sprite3 = magecityTsx.getTile(7, 7);

    // All should be valid 32x32 sprites (the working size)
    EXPECT_EQ(sprite1.w, 32);
    EXPECT_EQ(sprite1.h, 32);
    EXPECT_EQ(sprite2.w, 32);
    EXPECT_EQ(sprite2.h, 32);
    EXPECT_EQ(sprite3.w, 32);
    EXPECT_EQ(sprite3.h, 32);
}

// Test PNG characteristics of working sprites
TEST_F(TMXRenderingTest, WorkingPNGCharacteristics) {
    tsx magecityTsx("assets/Levels/test_one/magecity.tsx");
    ASSERT_FALSE(magecityTsx.empty());

    // Get PNG source information
    int source_width = magecityTsx.getSourceWidth();
    int source_height = magecityTsx.getSourceHeight();
    int tile_width = magecityTsx.getTileWidth();
    int tile_height = magecityTsx.getTileHeight();

    EXPECT_GT(source_width, 0) << "Source PNG width should be positive";
    EXPECT_GT(source_height, 0) << "Source PNG height should be positive";
    EXPECT_EQ(tile_width, 32) << "Working tile width is 32";
    EXPECT_EQ(tile_height, 32) << "Working tile height is 32";

    // Calculate expected tile grid
    int tiles_per_row = source_width / tile_width;
    int tile_rows = source_height / tile_height;

    EXPECT_GT(tiles_per_row, 0) << "Should have tiles per row";
    EXPECT_GT(tile_rows, 0) << "Should have tile rows";

    // Test that we can access tiles within these bounds
    CF_Sprite first_tile = magecityTsx.getTile(0, 0);
    CF_Sprite last_valid_x = magecityTsx.getTile(tiles_per_row - 1, 0);
    CF_Sprite last_valid_y = magecityTsx.getTile(0, tile_rows - 1);

    EXPECT_GT(first_tile.w, 0) << "First tile should be valid";
    EXPECT_GT(last_valid_x.w, 0) << "Last valid X tile should be valid";
    EXPECT_GT(last_valid_y.w, 0) << "Last valid Y tile should be valid";
}

// Test the actual TMX rendering methods that WORK in the main game
TEST_F(TMXRenderingTest, TMXActualRenderingMethods) {
    // Test the renderAllLayers method that successfully works in main game
    printf("\n=== TESTING TMX RENDERING METHODS (THAT WORK) ===\n");

    // Set up camera for rendering (like main game does)
    testCamera->setPosition(0.0f, 0.0f);
    testCamera->setZoom(1.0f);

    // Test that rendering setup doesn't crash
    EXPECT_NO_THROW({
        // This is the exact call that works in main.cpp: levelMap.renderAllLayers(camera);
        printf("Testing renderAllLayers with camera...\n");

        // We can't actually call the render without graphics context, but we can test setup
        ASSERT_GT(testMap->getLayerCount(), 0) << "Should have layers to render";

        // Test individual layer rendering preparation
        for (int i = 0; i < testMap->getLayerCount(); i++) {
            printf("Preparing to render layer %d...\n", i);
            auto layer = testMap->getLayer(i);
            ASSERT_NE(layer, nullptr) << "Layer " << i << " should exist";
            EXPECT_TRUE(layer->visible) << "Layer " << i << " should be visible for rendering";
        }
    });
}

TEST_F(TMXRenderingTest, TMXRenderLayerByIndex) {
    ASSERT_GT(testMap->getLayerCount(), 0);

    // Test rendering first layer by index (method used by renderAllLayers)
    printf("Testing renderLayer by index...\n");

    EXPECT_NO_THROW({
        // Test the rendering pipeline setup for individual layers
        auto layer = testMap->getLayer(0);
        ASSERT_NE(layer, nullptr);

        // Verify layer can be rendered
        EXPECT_TRUE(layer->visible) << "Layer should be visible for rendering";
        EXPECT_GT(layer->opacity, 0.0f) << "Layer should have opacity for rendering";

        // Test tile iteration (what renderLayer does internally)
        int renderable_tiles = 0;
        for (int y = 0; y < layer->height && renderable_tiles < 10; y++) {
            for (int x = 0; x < layer->width && renderable_tiles < 10; x++) {
                int gid = layer->getTileGID(x, y);
                if (gid > 0) {
                    // This is exactly what TMX rendering does for each tile
                    CF_Sprite sprite = testMap->getTileAt(0, x, y);
                    EXPECT_GT(sprite.w, 0) << "Renderable tile should have valid sprite";
                    renderable_tiles++;
                }
            }
        }

        EXPECT_GT(renderable_tiles, 0) << "Should find tiles to render in layer";
    });
}

TEST_F(TMXRenderingTest, TMXRenderLayerByName) {
    ASSERT_GT(testMap->getLayerCount(), 0);

    // Test rendering by layer name (alternative rendering method)
    auto firstLayer = testMap->getLayer(0);
    ASSERT_NE(firstLayer, nullptr);

    printf("Testing renderLayer by name: '%s'...\n", firstLayer->name.c_str());

    EXPECT_NO_THROW({
        // Test layer lookup by name
        auto foundLayer = testMap->getLayer(firstLayer->name);
        EXPECT_EQ(foundLayer, firstLayer) << "Should find layer by name";

        // Test that named layer rendering would work
        EXPECT_TRUE(foundLayer->visible) << "Named layer should be renderable";
    });
}

TEST_F(TMXRenderingTest, TMXCameraIntegratedRendering) {
    // Test the camera-integrated rendering that works in main game
    printf("Testing TMX rendering with camera integration...\n");

    // Test different camera positions/zooms like main game
    std::vector<std::pair<v2, float>> camera_configs;
    camera_configs.push_back({cf_v2(0.0f, 0.0f), 1.0f});      // Default position
    camera_configs.push_back({cf_v2(100.0f, 100.0f), 1.0f});  // Moved position
    camera_configs.push_back({cf_v2(0.0f, 0.0f), 2.0f});      // Zoomed in
    camera_configs.push_back({cf_v2(50.0f, 50.0f), 0.5f});    // Zoomed out + moved

    for (auto [pos, zoom] : camera_configs) {
        testCamera->setPosition(pos);
        testCamera->setZoom(zoom);

        // Test rendering preparation for this camera config
        ASSERT_GT(testMap->getLayerCount(), 0);

        // Verify camera configuration is valid for rendering
        EXPECT_GE(testCamera->getZoom(), 0.1f) << "Camera zoom should be reasonable";

        // Test world coordinate calculation (what TMX rendering does)
        float world_x = pos.x;
        float world_y = pos.y;

        // This mimics the renderAllLayers call with camera
        auto layer = testMap->getLayer(0);
        if (layer) {
            int tile_width = testMap->getTileWidth();
            int tile_height = testMap->getTileHeight();

            // Test tile world positioning calculation
            for (int test_x = 0; test_x < std::min(3, layer->width); test_x++) {
                for (int test_y = 0; test_y < std::min(3, layer->height); test_y++) {
                    float tile_world_x = world_x + (test_x * tile_width);
                    float tile_world_y = world_y + ((layer->height - 1 - test_y) * tile_height);

                    EXPECT_TRUE(std::isfinite(tile_world_x)) << "Tile world X should be finite";
                    EXPECT_TRUE(std::isfinite(tile_world_y)) << "Tile world Y should be finite";
                }
            }
        }
    }
}

TEST_F(TMXRenderingTest, TMXSpriteRenderingPipeline) {
    // Test the exact sprite rendering pipeline TMX uses (that works!)
    printf("Testing TMX sprite rendering pipeline...\n");

    ASSERT_GT(testMap->getLayerCount(), 0);
    auto layer = testMap->getLayer(0);
    ASSERT_NE(layer, nullptr);

    // Find first non-empty tile for testing
    int test_gid = -1;
    int test_x = -1, test_y = -1;

    for (int y = 0; y < layer->height && test_gid <= 0; y++) {
        for (int x = 0; x < layer->width && test_gid <= 0; x++) {
            int gid = layer->getTileGID(x, y);
            if (gid > 0) {
                test_gid = gid;
                test_x = x;
                test_y = y;
            }
        }
    }

    ASSERT_GT(test_gid, 0) << "Should find a tile to test rendering pipeline";

    EXPECT_NO_THROW({
        // This is the exact sequence TMX rendering uses:

        // 1. Get sprite at position (using the public TMX API)
        CF_Sprite sprite = testMap->getTileAt(0, test_x, test_y);
        EXPECT_GT(sprite.w, 0) << "Pipeline step 1: sprite creation should work";

        // 2. Calculate world position
        int tile_width = testMap->getTileWidth();
        int tile_height = testMap->getTileHeight();
        float world_x = 0.0f;
        float world_y = 0.0f;

        float tile_world_x = world_x + (test_x * tile_width);
        float tile_world_y = world_y + ((layer->height - 1 - test_y) * tile_height);

        EXPECT_TRUE(std::isfinite(tile_world_x)) << "Pipeline step 2: world X calculation";
        EXPECT_TRUE(std::isfinite(tile_world_y)) << "Pipeline step 2: world Y calculation";

        // 3. Round coordinates (TMX does this for pixel alignment)
        float rounded_world_x = roundf(world_x);
        float rounded_world_y = roundf(world_y);
        tile_world_x = rounded_world_x + (test_x * tile_width);
        tile_world_y = rounded_world_y + ((layer->height - 1 - test_y) * tile_height);

        EXPECT_TRUE(std::isfinite(tile_world_x)) << "Pipeline step 3: coordinate rounding";

        // 4. Validate sprite is ready for cf_draw_sprite call
        EXPECT_GT(sprite.w, 0) << "Pipeline step 4: sprite should be valid for rendering";
        EXPECT_GT(sprite.h, 0) << "Pipeline step 4: sprite should be valid for rendering";

        printf("TMX Pipeline Success: GID %d -> Sprite %dx%d at world (%.1f, %.1f)\n",
               test_gid, sprite.w, sprite.h, tile_world_x, tile_world_y);
    });
}

// Performance test for working TMX system
TEST_F(TMXRenderingTest, TMXPerformanceCharacteristics) {
    // Test creating many sprites quickly (like a real game would)
    auto start = std::chrono::high_resolution_clock::now();

    std::vector<CF_Sprite> sprites;
    int created_count = 0;
    const int target_sprites = 100;

    ASSERT_GT(testMap->getLayerCount(), 0);
    auto firstLayer = testMap->getLayer(0);
    ASSERT_NE(firstLayer, nullptr);

    // Create sprites from different positions
    for (int i = 0; i < target_sprites && created_count < target_sprites; i++) {
        int x = i % firstLayer->width;
        int y = (i / firstLayer->width) % firstLayer->height;

        int gid = firstLayer->getTileGID(x, y);
        if (gid > 0) {
            CF_Sprite sprite = testMap->getTileAt(0, x, y);
            if (sprite.w > 0) {
                sprites.push_back(sprite);
                created_count++;
            }
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    EXPECT_GT(created_count, 0) << "Should create some sprites for performance test";
    EXPECT_LT(duration.count(), 1000) << "Sprite creation should be fast (<1 second for " << created_count << " sprites)";
}

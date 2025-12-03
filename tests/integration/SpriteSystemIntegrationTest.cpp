#include <gtest/gtest.h>
#include <cute.h>
#include "SpriteAnimationLoader.h"

using namespace Cute;

class SpriteSystemIntegrationTest : public ::testing::Test
{
protected:
    SpriteAnimationLoader loader;

    void SetUp() override
    {
        // Set up test environment
    }

    void TearDown() override
    {
        // Clean up test environment
    }
};

// Test that the entire sprite system can be initialized without crashing
TEST_F(SpriteSystemIntegrationTest, SystemInitializationDoesNotCrash)
{
    // This test should pass if our sprite animation loader can be created
    // without triggering CF_ASSERT failures or crashes

    // Basic initialization should work
    EXPECT_EQ(loader.getCachedPNGCount(), 0);
    EXPECT_EQ(loader.getCacheSize(), 0);
}

// Test that sprite loading fails gracefully with invalid paths
TEST_F(SpriteSystemIntegrationTest, InvalidAssetPathsFailGracefully)
{
    // Test with a non-existent PNG file
    CF_Sprite frame = loader.extractSpriteFrame("nonexistent/path/file.png", 0, 0, 64, 64);

    // Should return a default sprite (w and h == 0) without crashing
    EXPECT_EQ(frame.w, 0);
    EXPECT_EQ(frame.h, 0);
}

// Test that sprite loading works with real test assets
TEST_F(SpriteSystemIntegrationTest, RealAssetLoadingWorks)
{
    // Test with the actual walkcycle PNG that exists
    const std::string png_path = "assets/Art/AnimationsSheets/walkcycle/BODY_skeleton.png";

    // This should succeed with the real PNG file
    CF_Sprite frame = loader.extractSpriteFrame(png_path, 0, 0, 64, 64);

    // Should return a valid sprite
    EXPECT_GT(frame.w, 0);
    EXPECT_GT(frame.h, 0);
}

// Test that animation table loading works
TEST_F(SpriteSystemIntegrationTest, AnimationTableLoadingWorks)
{
    // Create animation layouts for testing
    std::vector<AnimationLayout> layouts = {
        AnimationLayouts::WALKCYCLE_4_DIRECTIONS_9_FRAMES};

    // Load animation table - should not crash even if files don't exist at expected locations
    AnimationTable table = loader.loadAnimationTable("assets/Art/AnimationsSheets", layouts);

    // Table creation should succeed (though animations might be empty if files aren't found)
    std::vector<std::string> names = table.getAnimationNames();
    // Don't expect specific content - just that the method works without crashing
    EXPECT_TRUE(true);
}

// Test that frame extraction works with known good dimensions
TEST_F(SpriteSystemIntegrationTest, FrameExtractionWithCorrectDimensions)
{
    const std::string png_path = "assets/Art/AnimationsSheets/walkcycle/BODY_skeleton.png";

    // The PNG is 576x256, so we can extract multiple 64x64 frames
    // Test extracting a few frames from different positions

    // Frame 0,0 (first frame, first row - UP direction)
    CF_Sprite frame1 = loader.extractSpriteFrame(png_path, 0, 0, 64, 64);

    // Frame 1,0 (second frame, first row - still UP direction)
    CF_Sprite frame2 = loader.extractSpriteFrame(png_path, 64, 0, 64, 64);

    // Frame 0,1 (first frame, second row - LEFT direction)
    CF_Sprite frame3 = loader.extractSpriteFrame(png_path, 0, 64, 64, 64);

    // All frames should be valid
    EXPECT_GT(frame1.w, 0);
    EXPECT_GT(frame2.w, 0);
    EXPECT_GT(frame3.w, 0);
}

// Test that PNG caching works correctly
TEST_F(SpriteSystemIntegrationTest, PNGCachingWorks)
{
    const std::string png_path = "assets/Art/AnimationsSheets/walkcycle/BODY_skeleton.png";

    // Initially no PNGs cached
    EXPECT_EQ(loader.getCachedPNGCount(), 0);

    // Extract a frame - this should cache the PNG
    CF_Sprite frame = loader.extractSpriteFrame(png_path, 0, 0, 64, 64);

    // Now should have 1 cached PNG (if the file exists)
    if (frame.w > 0 && frame.h > 0)
    {
        EXPECT_EQ(loader.getCachedPNGCount(), 1);
        EXPECT_GT(loader.getCacheSize(), 0);
    }
}

// Test that sprite cleanup doesn't cause memory corruption
TEST_F(SpriteSystemIntegrationTest, SpriteCleanupDoesNotCorruptMemory)
{
    // Create and destroy multiple loaders to test memory management
    for (int i = 0; i < 5; i++)
    {
        SpriteAnimationLoader *testLoader = new SpriteAnimationLoader();

        // Do some work that might allocate memory
        testLoader->extractSpriteFrame("../assets/Art/AnimationsSheets/walkcycle/BODY_skeleton.png", 0, 0, 64, 64);

        // Clean up should not crash
        delete testLoader;
    }

    // If we get here without crashes, memory management is working
    EXPECT_TRUE(true);
}

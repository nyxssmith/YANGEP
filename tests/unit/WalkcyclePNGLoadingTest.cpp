#include <gtest/gtest.h>
#include <cute.h>
#include "../../src/lib/SpriteAnimationLoader.h"

using namespace Cute;

class WalkcyclePNGLoadingTest : public ::testing::Test {
protected:
    SpriteAnimationLoader loader;
    const std::string TEST_PNG_PATH = "assets/Art/AnimationsSheets/walkcycle/BODY_skeleton.png";

    void SetUp() override {
        // Test PNG should be 576x256 (9 columns x 4 rows of 64x64 tiles)
    }

    void TearDown() override {
        // Clean up
    }
};

// Test basic PNG loading without frame extraction
TEST_F(WalkcyclePNGLoadingTest, PNGFileCanBeLoaded) {
    // This tests the basic VFS PNG loading that has been proven to work in TSX
    bool loaded = loader.loadAndCachePNG(TEST_PNG_PATH);

    // Should succeed with real PNG file
    EXPECT_TRUE(loaded);
    EXPECT_EQ(loader.getCachedPNGCount(), 1);
    EXPECT_GT(loader.getCacheSize(), 0);
}

// Test extracting the first frame (UP direction, frame 0)
TEST_F(WalkcyclePNGLoadingTest, CanExtractFirstFrame) {
    // First frame at top-left corner
    CF_Sprite frame = loader.extractSpriteFrame(TEST_PNG_PATH, 0, 0, 64, 64);

    // Should get a valid sprite
    EXPECT_GT(frame.w, 0);
    EXPECT_EQ(frame.w, 64);
    EXPECT_EQ(frame.h, 64);
}

// Test extracting frames from each direction (each row)
TEST_F(WalkcyclePNGLoadingTest, CanExtractFramesFromEachDirection) {
    // The sprite sheet has 4 rows representing different directions:
    // Row 0: UP direction
    // Row 1: LEFT direction
    // Row 2: DOWN direction
    // Row 3: RIGHT direction

    // Extract first frame from each row
    CF_Sprite up_frame = loader.extractSpriteFrame(TEST_PNG_PATH, 0, 0, 64, 64);      // Row 0
    CF_Sprite left_frame = loader.extractSpriteFrame(TEST_PNG_PATH, 0, 64, 64, 64);   // Row 1
    CF_Sprite down_frame = loader.extractSpriteFrame(TEST_PNG_PATH, 0, 128, 64, 64);  // Row 2
    CF_Sprite right_frame = loader.extractSpriteFrame(TEST_PNG_PATH, 0, 192, 64, 64); // Row 3

    // All should be valid
    EXPECT_GT(up_frame.w, 0);
    EXPECT_GT(left_frame.w, 0);
    EXPECT_GT(down_frame.w, 0);
    EXPECT_GT(right_frame.w, 0);

    // All should be 64x64
    EXPECT_EQ(up_frame.w, 64);    EXPECT_EQ(up_frame.h, 64);
    EXPECT_EQ(left_frame.w, 64);  EXPECT_EQ(left_frame.h, 64);
    EXPECT_EQ(down_frame.w, 64);  EXPECT_EQ(down_frame.h, 64);
    EXPECT_EQ(right_frame.w, 64); EXPECT_EQ(right_frame.h, 64);
}

// Test extracting multiple frames from one direction (animation sequence)
TEST_F(WalkcyclePNGLoadingTest, CanExtractAnimationSequence) {
    // Each direction has 9 frames (columns), let's test the UP direction (row 0)
    std::vector<CF_Sprite> frames;

    for (int frame = 0; frame < 9; frame++) {
        int pixel_x = frame * 64;  // Each frame is 64 pixels wide
        int pixel_y = 0;           // Row 0 for UP direction

        CF_Sprite sprite = loader.extractSpriteFrame(TEST_PNG_PATH, pixel_x, pixel_y, 64, 64);
        frames.push_back(sprite);

        // Each frame should be valid
        EXPECT_GT(sprite.w, 0);
        EXPECT_EQ(sprite.w, 64);
        EXPECT_EQ(sprite.h, 64);
    }

    // Should have extracted all 9 frames
    EXPECT_EQ(frames.size(), 9);
}

// Test bounds checking - extracting beyond image boundaries should fail gracefully
TEST_F(WalkcyclePNGLoadingTest, BoundsCheckingWorks) {
    // PNG is 576x256, so extracting at position (576, 256) should fail
    CF_Sprite invalid_frame = loader.extractSpriteFrame(TEST_PNG_PATH, 576, 256, 64, 64);

    // Should return default sprite (invalid)
    EXPECT_EQ(invalid_frame.w, 0);
    EXPECT_EQ(invalid_frame.h, 0);
}

// Test loading complete animation using the layout system
TEST_F(WalkcyclePNGLoadingTest, CanLoadCompleteAnimation) {
    // Use the predefined walkcycle layout
    const AnimationLayout &layout = AnimationLayouts::WALKCYCLE_4_DIRECTIONS_9_FRAMES;

    // Load all frames using the layout system
    std::vector<CF_Sprite> frames = loader.loadAnimationFrames(TEST_PNG_PATH, layout);

    // Should load 4 directions × 9 frames = 36 total frames
    EXPECT_EQ(frames.size(), 36);

    // All frames should be valid
    for (const auto &frame : frames) {
        EXPECT_GT(frame.w, 0);
        EXPECT_EQ(frame.w, 64);
        EXPECT_EQ(frame.h, 64);
    }
}

// Test creating a complete Animation object
TEST_F(WalkcyclePNGLoadingTest, CanCreateCompleteAnimation) {
    const AnimationLayout &layout = AnimationLayouts::WALKCYCLE_4_DIRECTIONS_9_FRAMES;

    // Create complete animation
    Animation animation = loader.createAnimation("walkcycle", TEST_PNG_PATH, layout);

    // Should have created a valid animation
    EXPECT_EQ(animation.name, "walkcycle");
    EXPECT_EQ(animation.frames.size(), 36);  // 4 directions × 9 frames
    EXPECT_TRUE(animation.looping);
    EXPECT_GT(animation.totalDuration, 0);

    // Test frame access by direction and index
    const AnimationFrame* up_frame0 = animation.getFrame(0, Direction::UP);
    const AnimationFrame* left_frame0 = animation.getFrame(0, Direction::LEFT);

    EXPECT_NE(up_frame0, nullptr);
    EXPECT_NE(left_frame0, nullptr);
    EXPECT_EQ(up_frame0->direction, Direction::UP);
    EXPECT_EQ(left_frame0->direction, Direction::LEFT);
}

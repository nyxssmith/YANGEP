#include <gtest/gtest.h>
#include <cute.h>
#include "../../src/lib/Animation.h"

using namespace Cute;

class AnimationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up test environment
    }

    void TearDown() override {
        // Clean up test environment
    }
};

// Test Animation creation and basic properties
TEST_F(AnimationTest, CreateAnimation) {
    Animation anim("test_animation");

    EXPECT_EQ(anim.getName(), "test_animation");
    EXPECT_EQ(anim.getFrameCount(), 0);
    EXPECT_EQ(anim.getCurrentFrame(), 0);
    EXPECT_FALSE(anim.isPlaying());
    EXPECT_EQ(anim.getSpeed(), 1.0f);
    v2 expectedFrameSize(64, 256);
    v2 expectedSheetSize(64, 256);
    EXPECT_EQ(anim.getFrameSize().x, expectedFrameSize.x);
    EXPECT_EQ(anim.getFrameSize().y, expectedFrameSize.y);
    EXPECT_EQ(anim.getSheetSize().x, expectedSheetSize.x);
    EXPECT_EQ(anim.getSheetSize().y, expectedSheetSize.y);
}

// Test Animation creation with sprite sheet properties
TEST_F(AnimationTest, CreateAnimationWithSpriteSheet) {
    v2 frameSize(64, 256);
    v2 sheetSize(576, 256); // Walk cycle sprite sheet size
    Animation anim("walk_cycle", frameSize, sheetSize);

    EXPECT_EQ(anim.getName(), "walk_cycle");
    EXPECT_EQ(anim.getFrameSize().x, frameSize.x);
    EXPECT_EQ(anim.getFrameSize().y, frameSize.y);
    EXPECT_EQ(anim.getSheetSize().x, sheetSize.x);
    EXPECT_EQ(anim.getSheetSize().y, sheetSize.y);
}

// Test adding frames to animation
TEST_F(AnimationTest, AddFrames) {
    Animation anim("test_animation");

    // Create test frames with sprite sheet coordinates
    AnimationFrame frame1 = {0, 0, 0.1f, "frame1"}; // First frame at (0,0)
    AnimationFrame frame2 = {1, 0, 0.1f, "frame2"}; // Second frame at (1,0)

    anim.addFrame(frame1);
    anim.addFrame(frame2);

    EXPECT_EQ(anim.getFrameCount(), 2);
    EXPECT_EQ(anim.getFrame(0).frameX, 0);
    EXPECT_EQ(anim.getFrame(0).frameY, 0);
    EXPECT_EQ(anim.getFrame(1).frameX, 1);
    EXPECT_EQ(anim.getFrame(1).frameY, 0);
}

// Test adding frame sequence for walk cycle
TEST_F(AnimationTest, AddFrameSequence) {
    v2 frameSize(64, 256);
    v2 sheetSize(576, 256); // 9 frames × 64 pixels wide
    Animation anim("walk_cycle", frameSize, sheetSize);

    // Add 9 frames horizontally (like the real walk cycle)
    anim.addFrameSequence(0, 9, 0.1f, true);

    EXPECT_EQ(anim.getFrameCount(), 9);
    EXPECT_EQ(anim.getFrame(0).frameX, 0);
    EXPECT_EQ(anim.getFrame(0).frameY, 0);
    EXPECT_EQ(anim.getFrame(8).frameX, 8);
    EXPECT_EQ(anim.getFrame(8).frameY, 0);
}

// Test animation playback
TEST_F(AnimationTest, PlayAnimation) {
    Animation anim("test_animation");

    // Add frames with sprite sheet coordinates
    AnimationFrame frame1 = {0, 0, 0.1f, "frame1"};
    AnimationFrame frame2 = {1, 0, 0.1f, "frame2"};
    anim.addFrame(frame1);
    anim.addFrame(frame2);

    // Start playing
    anim.play();
    EXPECT_TRUE(anim.isPlaying());
    EXPECT_EQ(anim.getCurrentFrame(), 0);

    // Update animation
    anim.update(0.05f); // Half way through first frame
    EXPECT_EQ(anim.getCurrentFrame(), 0);

    anim.update(0.1f); // Complete first frame
    EXPECT_EQ(anim.getCurrentFrame(), 1);

    anim.update(0.1f); // Complete second frame
    EXPECT_EQ(anim.getCurrentFrame(), 0); // Should loop
}

// Test animation speed control
TEST_F(AnimationTest, AnimationSpeed) {
    Animation anim("test_animation");

    AnimationFrame frame = {0, 0, 0.1f, "frame1"};
    anim.addFrame(frame);
    anim.setSpeed(2.0f); // Double speed

    anim.play();
    anim.update(0.05f); // Should advance to next frame at double speed

    EXPECT_EQ(anim.getCurrentFrame(), 0); // Still on first frame
    anim.update(0.05f); // Complete frame at double speed
    EXPECT_EQ(anim.getCurrentFrame(), 0); // Looped back
}

// Test animation state management
TEST_F(AnimationTest, AnimationStates) {
    Animation anim("test_animation");

    AnimationFrame frame = {0, 0, 0.1f, "frame1"};
    anim.addFrame(frame);

    // Test play/pause/stop
    anim.play();
    EXPECT_TRUE(anim.isPlaying());

    anim.pause();
    EXPECT_FALSE(anim.isPlaying());

    anim.play();
    EXPECT_TRUE(anim.isPlaying());

    anim.stop();
    EXPECT_FALSE(anim.isPlaying());
    EXPECT_EQ(anim.getCurrentFrame(), 0); // Should reset to beginning
}

// Test frame timing accuracy
TEST_F(AnimationTest, FrameTiming) {
    Animation anim("test_animation");

    // Add frames with different durations
    AnimationFrame frame1 = {0, 0, 0.2f, "frame1"}; // 200ms
    AnimationFrame frame2 = {1, 0, 0.1f, "frame2"}; // 100ms
    anim.addFrame(frame1);
    anim.addFrame(frame2);

    anim.play();

    // First frame should last 200ms
    anim.update(0.1f);
    EXPECT_EQ(anim.getCurrentFrame(), 0);

    anim.update(0.1f);
    EXPECT_EQ(anim.getCurrentFrame(), 1);

    // Second frame should last 100ms
    anim.update(0.1f);
    EXPECT_EQ(anim.getCurrentFrame(), 0); // Looped back
}

// Test UV coordinate calculation
TEST_F(AnimationTest, UVCoordinates) {
    v2 frameSize(64, 256);
    v2 sheetSize(576, 256); // 9 frames × 64 pixels wide
    Animation anim("walk_cycle", frameSize, sheetSize);

    // Add 9 frames horizontally
    anim.addFrameSequence(0, 9, 0.1f, true);

    // Start playing the animation
    anim.play();

    // Test UV coordinates for first frame
    v2 uv1 = anim.getCurrentFrameUV();
    EXPECT_EQ(uv1.x, 0.0f); // First frame starts at x=0
    EXPECT_EQ(uv1.y, 0.0f); // First frame starts at y=0

    // Test UV coordinates for middle frame
    anim.update(0.1f); // Advance to second frame
    v2 uv2 = anim.getCurrentFrameUV();
    EXPECT_EQ(uv2.x, 64.0f / 576.0f); // Second frame at x=64
    EXPECT_EQ(uv2.y, 0.0f); // Still at y=0

    // Test UV coordinates for last frame
    for (int i = 0; i < 7; i++) {  // Only need 7 updates to reach frame 8 (last frame)
        anim.update(0.1f); // Advance to last frame
    }
    v2 uvLast = anim.getCurrentFrameUV();
    EXPECT_EQ(uvLast.x, 8.0f * 64.0f / 576.0f); // Last frame at x=8*64
    EXPECT_EQ(uvLast.y, 0.0f); // Still at y=0
}

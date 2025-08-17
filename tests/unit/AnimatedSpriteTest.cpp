#include <gtest/gtest.h>
#include <cute.h>
#include "../../src/lib/Sprite.h"
#include "../../src/lib/Animation.h"
#include "../../src/lib/AnimatedSprite.h"

using namespace Cute;

class AnimatedSpriteTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up test environment
    }

    void TearDown() override {
        // Clean up test environment
    }
};

// Test animated sprite creation
TEST_F(AnimatedSpriteTest, CreateAnimatedSprite) {
    AnimatedSprite sprite(std::string("test_sprite"));

    EXPECT_EQ(sprite.getName(), "test_sprite");
    EXPECT_EQ(sprite.getAnimationCount(), 0);
    EXPECT_FALSE(sprite.hasCurrentAnimation());
    EXPECT_EQ(sprite.getCurrentAnimationName(), "");
}

// Test adding animations to sprite
TEST_F(AnimatedSpriteTest, AddAnimations) {
    AnimatedSprite sprite(std::string("test_sprite"));

    // Create and add animations
    Animation idle_anim("idle");
    Animation walk_anim("walk");

    sprite.addAnimation(idle_anim);
    sprite.addAnimation(walk_anim);

    EXPECT_EQ(sprite.getAnimationCount(), 2);
    EXPECT_TRUE(sprite.hasAnimation("idle"));
    EXPECT_TRUE(sprite.hasAnimation("walk"));
    EXPECT_FALSE(sprite.hasAnimation("run"));
}

// Test playing animations
TEST_F(AnimatedSpriteTest, PlayAnimation) {
    AnimatedSprite sprite(std::string("test_sprite"));

    // Create animation with frames
    Animation idle_anim("idle");
    AnimationFrame frame1 = {0, 0, 0.1f, "idle_frame1"};
    AnimationFrame frame2 = {1, 0, 0.1f, "idle_frame2"};
    idle_anim.addFrame(frame1);
    idle_anim.addFrame(frame2);

    sprite.addAnimation(idle_anim);

    // Play the animation
    EXPECT_TRUE(sprite.playAnimation("idle"));
    EXPECT_TRUE(sprite.hasCurrentAnimation());
    EXPECT_EQ(sprite.getCurrentAnimationName(), "idle");
    EXPECT_EQ(sprite.getCurrentFrameIndex(), 0);
}

// Test animation state transitions
TEST_F(AnimatedSpriteTest, AnimationTransitions) {
    AnimatedSprite sprite(std::string("test_sprite"));

    // Create multiple animations
    Animation idle_anim("idle");
    Animation walk_anim("walk");

    // Add frames to idle
    AnimationFrame idle_frame = {0, 0, 0.1f, "idle_frame"};
    idle_anim.addFrame(idle_frame);

    // Add frames to walk
    AnimationFrame walk_frame1 = {0, 0, 0.1f, "walk_frame1"};
    AnimationFrame walk_frame2 = {1, 0, 0.1f, "walk_frame2"};
    walk_anim.addFrame(walk_frame1);
    walk_anim.addFrame(walk_frame2);

    sprite.addAnimation(idle_anim);
    sprite.addAnimation(walk_anim);

    // Start with idle
    sprite.playAnimation("idle");
    EXPECT_EQ(sprite.getCurrentAnimationName(), "idle");

    // Switch to walk
    sprite.playAnimation("walk");
    EXPECT_EQ(sprite.getCurrentAnimationName(), "walk");
    EXPECT_EQ(sprite.getCurrentFrameIndex(), 0);
}

// Test animation updates
TEST_F(AnimatedSpriteTest, AnimationUpdates) {
    AnimatedSprite sprite(std::string("test_sprite"));

    // Create animation with multiple frames
    Animation walk_anim("walk");
    AnimationFrame frame1 = {0, 0, 0.1f, "walk_frame1"};
    AnimationFrame frame2 = {1, 0, 0.1f, "walk_frame2"};
    walk_anim.addFrame(frame1);
    walk_anim.addFrame(frame2);

    sprite.addAnimation(walk_anim);
    sprite.playAnimation("walk");

    // Update animation
    sprite.update(0.05f); // Half way through first frame
    EXPECT_EQ(sprite.getCurrentFrameIndex(), 0);

    sprite.update(0.1f); // Complete first frame
    EXPECT_EQ(sprite.getCurrentFrameIndex(), 1);

    sprite.update(0.1f); // Complete second frame, should loop
    EXPECT_EQ(sprite.getCurrentFrameIndex(), 0);
}

// Test animation speed control
TEST_F(AnimatedSpriteTest, AnimationSpeed) {
    AnimatedSprite sprite(std::string("test_sprite"));

    // Create animation
    Animation anim("test");
    AnimationFrame frame = {0, 0, 0.1f, "frame"};
    anim.addFrame(frame);

    sprite.addAnimation(anim);
    sprite.playAnimation("test");

    // Set animation speed
    sprite.setAnimationSpeed("test", 2.0f); // Double speed

    // Update at double speed
    sprite.update(0.05f); // Should complete frame at double speed
    EXPECT_EQ(sprite.getCurrentFrameIndex(), 0); // Looped back
}

// Test stopping animations
TEST_F(AnimatedSpriteTest, StopAnimation) {
    AnimatedSprite sprite(std::string("test_sprite"));

    // Create and play animation
    Animation anim("test");
    AnimationFrame frame = {0, 0, 0.1f, "frame"};
    anim.addFrame(frame);

    sprite.addAnimation(anim);
    sprite.playAnimation("test");

    EXPECT_TRUE(sprite.hasCurrentAnimation());

    // Stop animation
    sprite.stopAnimation();
    EXPECT_FALSE(sprite.hasCurrentAnimation());
    EXPECT_EQ(sprite.getCurrentAnimationName(), "");
}

// Test animation completion callbacks
TEST_F(AnimatedSpriteTest, AnimationCallbacks) {
    AnimatedSprite sprite(std::string("test_sprite"));

    // Create animation
    Animation anim("test");
    AnimationFrame frame = {0, 0, 0.1f, "frame"};
    anim.addFrame(frame);
    anim.setLooping(false);  // Make it non-looping so it completes

    sprite.addAnimation(anim);

    // Set completion callback
    bool callback_called = false;
    sprite.setOnAnimationComplete([&callback_called](const std::string& anim_name) {
        callback_called = true;
        EXPECT_EQ(anim_name, "test");
    });

    // Play and complete animation
    sprite.playAnimation("test");
    sprite.update(0.1f); // Complete the frame

    EXPECT_TRUE(callback_called);
}

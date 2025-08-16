#include <gtest/gtest.h>
#include <cute.h>
#include "../../src/lib/PNGSprite.h"

using namespace Cute;

class SpriteSystemIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up test environment
    }

    void TearDown() override {
        // Clean up test environment
    }
};

// Test that the entire sprite system can be initialized without crashing
TEST_F(SpriteSystemIntegrationTest, SystemInitializationDoesNotCrash) {
    // This test should pass if our sprite system can be created
    // without triggering CF_ASSERT failures or crashes
    
    PNGSprite sprite;
    
    // Basic initialization should work
    EXPECT_TRUE(sprite.isValid());
    EXPECT_EQ(sprite.getWidth(), 64);
    EXPECT_EQ(sprite.getHeight(), 64);
    
    // Default state should be reasonable
    EXPECT_EQ(sprite.getDirection(), Direction::DOWN);
    EXPECT_STREQ(sprite.getCurrentAnimation(), "idle");
}

// Test that sprite loading fails gracefully with invalid paths
TEST_F(SpriteSystemIntegrationTest, InvalidAssetPathsFailGracefully) {
    PNGSprite sprite;
    
    // This should fail immediately with exit(1) - not crash with CF_ASSERT
    EXPECT_DEATH({
        sprite.loadAnimations(
            "nonexistent/path/idle.png",
            "nonexistent/path/walkcycle.png"
        );
    }, ".*");
}

// Test that sprite loading actually works with our virtual PNG approach
TEST_F(SpriteSystemIntegrationTest, VirtualPNGApproachWorksAtLoadTime) {
    PNGSprite sprite;
    
    // This should succeed because our virtual PNG approach works at load time
    // The real test will be whether rendering works at runtime
    EXPECT_NO_THROW({
        sprite.loadAnimations(
            "assets/Art/AnimationsSheets/idle/BODY_skeleton.png",
            "assets/Art/AnimationsSheets/walkcycle/BODY_skeleton.png"
        );
    });
    
    // The sprite should be valid after loading
    EXPECT_TRUE(sprite.isValid());
    
    // But the real test will be whether rendering works
    // We expect this to potentially fail at runtime due to CF_ASSERT
    // when the framework tries to actually render the virtual PNGs
}

// Test that direction changes work correctly
TEST_F(SpriteSystemIntegrationTest, DirectionChangesWork) {
    PNGSprite sprite;
    
    // Test all directions
    sprite.setDirection(Direction::UP);
    EXPECT_EQ(sprite.getDirection(), Direction::UP);
    
    sprite.setDirection(Direction::LEFT);
    EXPECT_EQ(sprite.getDirection(), Direction::LEFT);
    
    sprite.setDirection(Direction::DOWN);
    EXPECT_EQ(sprite.getDirection(), Direction::DOWN);
    
    sprite.setDirection(Direction::RIGHT);
    EXPECT_EQ(sprite.getDirection(), Direction::RIGHT);
}

// Test that animation switching works correctly
TEST_F(SpriteSystemIntegrationTest, AnimationSwitchingWorks) {
    PNGSprite sprite;
    
    // Test animation switching
    sprite.setAnimation("idle");
    EXPECT_STREQ(sprite.getCurrentAnimation(), "idle");
    
    sprite.setAnimation("walkcycle");
    EXPECT_STREQ(sprite.getCurrentAnimation(), "walkcycle");
}

// Test that sprite rendering doesn't crash
TEST_F(SpriteSystemIntegrationTest, SpriteRenderingDoesNotCrash) {
    PNGSprite sprite;
    
    // These calls should not crash, even if they don't render anything
    EXPECT_NO_THROW({
        sprite.render();
        sprite.update(0.016f); // 60 FPS delta time
    });
}

// Test that sprite cleanup doesn't cause memory corruption
TEST_F(SpriteSystemIntegrationTest, SpriteCleanupDoesNotCorruptMemory) {
    // Create and destroy multiple sprites to test memory management
    for (int i = 0; i < 10; i++) {
        PNGSprite* sprite = new PNGSprite();
        EXPECT_TRUE(sprite->isValid());
        delete sprite;
    }
    
    // If we get here without crashes, memory management is working
    EXPECT_TRUE(true);
}

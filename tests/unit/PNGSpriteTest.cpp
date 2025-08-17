#include <gtest/gtest.h>
#include <cute.h>
#include "../../src/lib/PNGSprite.h"

using namespace Cute;

class PNGSpriteTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up test environment
    }

    void TearDown() override {
        // Clean up test environment
    }
};

// Test that sprite loading actually works (contrary to our expectations!)
TEST_F(PNGSpriteTest, SpriteLoadingActuallyWorks) {
    // This test reveals that our virtual PNG approach is actually working
    // at load time, which means the CF_ASSERT failures will happen at runtime

    PNGSprite sprite;

    // The loadAnimations call should actually succeed now
    // The real test will be whether rendering works at runtime
    EXPECT_NO_THROW({
        sprite.loadAnimations(
            "assets/Art/AnimationsSheets/idle/BODY_skeleton.png",
            "assets/Art/AnimationsSheets/walkcycle/BODY_skeleton.png"
        );
    });

    // The sprite should appear valid after loading
    EXPECT_TRUE(sprite.isValid());
}

// Test that sprites are created with correct dimensions
TEST_F(PNGSpriteTest, SpriteDimensionsAreCorrect) {
    PNGSprite sprite;

    // Before loading, dimensions should be default
    EXPECT_EQ(sprite.getWidth(), 64);
    EXPECT_EQ(sprite.getHeight(), 64);

    // After loading, dimensions should still be 64x64 (single frame)
    sprite.loadAnimations(
        "assets/Art/AnimationsSheets/idle/BODY_skeleton.png",
        "assets/Art/AnimationsSheets/walkcycle/BODY_skeleton.png"
    );

    EXPECT_EQ(sprite.getWidth(), 64);
    EXPECT_EQ(sprite.getHeight(), 64);
}

// Test that animation switching works correctly
TEST_F(PNGSpriteTest, AnimationSwitchingWorks) {
    PNGSprite sprite;

    sprite.loadAnimations(
        "assets/Art/AnimationsSheets/idle/BODY_skeleton.png",
        "assets/Art/AnimationsSheets/walkcycle/BODY_skeleton.png"
    );

    // Test animation switching
    sprite.setAnimation("idle");
    EXPECT_STREQ(sprite.getCurrentAnimation(), "idle");

    sprite.setAnimation("walkcycle");
    EXPECT_STREQ(sprite.getCurrentAnimation(), "walkcycle");
}

// Test that direction changes work correctly
TEST_F(PNGSpriteTest, DirectionChangesWork) {
    PNGSprite sprite;

    sprite.loadAnimations(
        "assets/Art/AnimationsSheets/idle/BODY_skeleton.png",
        "assets/Art/AnimationsSheets/walkcycle/BODY_skeleton.png"
    );

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

// Test that invalid asset paths cause immediate failure
TEST_F(PNGSpriteTest, InvalidAssetPathsCauseImmediateFailure) {
    PNGSprite sprite;

    // This should fail immediately with exit(1)
    EXPECT_DEATH({
        sprite.loadAnimations(
            "nonexistent/path/idle.png",
            "nonexistent/path/walkcycle.png"
        );
    }, ".*");
}

// Test that our virtual PNG approach works at load time
TEST_F(PNGSpriteTest, VirtualPNGApproachWorksAtLoadTime) {
    PNGSprite sprite;

    // This should succeed because our virtual PNG approach works
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

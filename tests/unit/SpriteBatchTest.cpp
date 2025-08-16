#include <gtest/gtest.h>
#include "../../src/lib/SpriteBatch.h"

using namespace Cute;

class SpriteBatchTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up any common test data
    }

    void TearDown() override {
        // Clean up any test data
    }
};

// Test constructors
TEST_F(SpriteBatchTest, DefaultConstructor) {
    SpriteBatch spriteBatch;
    EXPECT_EQ(spriteBatch.getDirection(), Direction::DOWN);
    EXPECT_EQ(spriteBatch.getFrame(), 0);
    EXPECT_EQ(spriteBatch.getFrameCount(), 1);

    v2 frameSize = spriteBatch.getFrameSize();
    EXPECT_EQ(frameSize.x, 64.0f);
    EXPECT_EQ(frameSize.y, 64.0f);

    v2 renderScale = spriteBatch.getRenderScale();
    EXPECT_EQ(renderScale.x, 1.0f);
    EXPECT_EQ(renderScale.y, 1.0f);
}

TEST_F(SpriteBatchTest, ConstructorWithTexturePath) {
    // Test loading a valid sprite from test assets
    EXPECT_NO_THROW({
        SpriteBatch spriteBatch("tests/assets/skeleton/BODY_skeleton.png");
        EXPECT_EQ(spriteBatch.getDirection(), Direction::DOWN);
        EXPECT_EQ(spriteBatch.getFrame(), 0);
        EXPECT_EQ(spriteBatch.getFrameCount(), 1);

        v2 frameSize = spriteBatch.getFrameSize();
        EXPECT_EQ(frameSize.x, 64.0f);
        EXPECT_EQ(frameSize.y, 64.0f);
    });
}

TEST_F(SpriteBatchTest, ConstructorWithFrameSpecifications) {
    SpriteBatch spriteBatch("tests/assets/skeleton/BODY_skeleton.png", 4, v2(64, 64));
    EXPECT_EQ(spriteBatch.getFrameCount(), 4);

    v2 frameSize = spriteBatch.getFrameSize();
    EXPECT_EQ(frameSize.x, 64.0f);
    EXPECT_EQ(frameSize.y, 64.0f);
}

TEST_F(SpriteBatchTest, ConstructorWithRenderScale) {
    SpriteBatch spriteBatch("tests/assets/skeleton/BODY_skeleton.png", 4, v2(64, 64), v2(2.0f, 2.0f));

    v2 renderScale = spriteBatch.getRenderScale();
    EXPECT_EQ(renderScale.x, 2.0f);
    EXPECT_EQ(renderScale.y, 2.0f);
}

// Test direction management
TEST_F(SpriteBatchTest, DirectionManagement) {
    SpriteBatch spriteBatch;

    spriteBatch.setDirection(Direction::UP);
    EXPECT_EQ(spriteBatch.getDirection(), Direction::UP);

    spriteBatch.setDirection(Direction::LEFT);
    EXPECT_EQ(spriteBatch.getDirection(), Direction::LEFT);

    spriteBatch.setDirection(Direction::DOWN);
    EXPECT_EQ(spriteBatch.getDirection(), Direction::DOWN);

    spriteBatch.setDirection(Direction::RIGHT);
    EXPECT_EQ(spriteBatch.getDirection(), Direction::RIGHT);
}

// Test frame management
TEST_F(SpriteBatchTest, FrameManagement) {
    SpriteBatch spriteBatch("tests/assets/skeleton/BODY_skeleton.png", 4, v2(64, 64));

    EXPECT_EQ(spriteBatch.getFrame(), 0);
    EXPECT_EQ(spriteBatch.getFrameCount(), 4);

    spriteBatch.setFrame(2);
    EXPECT_EQ(spriteBatch.getFrame(), 2);

    spriteBatch.setFrame(5); // Should clamp to valid range
    EXPECT_EQ(spriteBatch.getFrame(), 3); // Should be frameCount - 1

    spriteBatch.setFrame(-1); // Should clamp to valid range
    EXPECT_EQ(spriteBatch.getFrame(), 0);
}

// Test frame navigation
TEST_F(SpriteBatchTest, FrameNavigation) {
    SpriteBatch spriteBatch("tests/assets/skeleton/BODY_skeleton.png", 4, v2(64, 64));

    EXPECT_EQ(spriteBatch.getFrame(), 0);

    spriteBatch.nextFrame();
    EXPECT_EQ(spriteBatch.getFrame(), 1);

    spriteBatch.nextFrame();
    EXPECT_EQ(spriteBatch.getFrame(), 2);

    spriteBatch.nextFrame();
    EXPECT_EQ(spriteBatch.getFrame(), 3);

    spriteBatch.nextFrame(); // Should wrap around
    EXPECT_EQ(spriteBatch.getFrame(), 0);

    spriteBatch.previousFrame(); // Should wrap around
    EXPECT_EQ(spriteBatch.getFrame(), 3);

    spriteBatch.resetFrame();
    EXPECT_EQ(spriteBatch.getFrame(), 0);
}

// Test frame size management
TEST_F(SpriteBatchTest, FrameSizeManagement) {
    SpriteBatch spriteBatch;

    v2 newSize(32, 32);
    spriteBatch.setFrameSize(newSize);
    v2 frameSize = spriteBatch.getFrameSize();
    EXPECT_EQ(frameSize.x, 32.0f);
    EXPECT_EQ(frameSize.y, 32.0f);

    v2 anotherSize(128, 128);
    spriteBatch.setFrameSize(anotherSize);
    frameSize = spriteBatch.getFrameSize();
    EXPECT_EQ(frameSize.x, 128.0f);
    EXPECT_EQ(frameSize.y, 128.0f);
}

// Test render scale management
TEST_F(SpriteBatchTest, RenderScaleManagement) {
    SpriteBatch spriteBatch;

    v2 newScale(2.0f, 2.0f);
    spriteBatch.setRenderScale(newScale);
    v2 renderScale = spriteBatch.getRenderScale();
    EXPECT_EQ(renderScale.x, 2.0f);
    EXPECT_EQ(renderScale.y, 2.0f);

    v2 anotherScale(0.5f, 0.5f);
    spriteBatch.setRenderScale(anotherScale);
    renderScale = spriteBatch.getRenderScale();
    EXPECT_EQ(renderScale.x, 0.5f);
    EXPECT_EQ(renderScale.y, 0.5f);
}

// Test reset functionality
TEST_F(SpriteBatchTest, ResetFunctionality) {
    SpriteBatch spriteBatch("tests/assets/skeleton/BODY_skeleton.png", 4, v2(64, 64));

    // Change some state
    spriteBatch.setDirection(Direction::RIGHT);
    spriteBatch.setFrame(2);

    // Reset
    spriteBatch.resetFrame();

    // Check reset state
    EXPECT_EQ(spriteBatch.getFrame(), 0);
    // Direction should remain the same (reset only affects frame)
    EXPECT_EQ(spriteBatch.getDirection(), Direction::RIGHT);
}

// Test UV coordinate calculation
TEST_F(SpriteBatchTest, UVCoordinateCalculation) {
    SpriteBatch spriteBatch("tests/assets/skeleton/BODY_skeleton.png", 4, v2(64, 64));

    // Test UV calculation for different directions and frames
    v2 uv1 = spriteBatch.calculateFrameUV(0, Direction::UP);
    EXPECT_EQ(uv1.x, 0.0f);
    EXPECT_EQ(uv1.y, 0.0f);

    v2 uv2 = spriteBatch.calculateFrameUV(0, Direction::LEFT);
    EXPECT_EQ(uv2.x, 0.0f);
    EXPECT_EQ(uv2.y, 0.25f);

    v2 uv3 = spriteBatch.calculateFrameUV(0, Direction::DOWN);
    EXPECT_EQ(uv3.x, 0.0f);
    EXPECT_EQ(uv3.y, 0.5f);

    v2 uv4 = spriteBatch.calculateFrameUV(0, Direction::RIGHT);
    EXPECT_EQ(uv4.x, 0.0f);
    EXPECT_EQ(uv4.y, 0.75f);
}

// Test render and update methods
TEST_F(SpriteBatchTest, RenderAndUpdate) {
    SpriteBatch spriteBatch("tests/assets/skeleton/BODY_skeleton.png", 4, v2(64, 64));

    // These methods should not crash
    EXPECT_NO_THROW(spriteBatch.render());
    EXPECT_NO_THROW(spriteBatch.update(0.016f));
}

// Test inheritance from Sprite
TEST_F(SpriteBatchTest, InheritanceFromSprite) {
    SpriteBatch spriteBatch("tests/assets/skeleton/BODY_skeleton.png", 4, v2(64, 64));

    // Test that we can call base Sprite methods
    EXPECT_NO_THROW(spriteBatch.setPosition(v2(100, 100)));
    EXPECT_NO_THROW(spriteBatch.setScale(v2(2.0f, 2.0f)));
    EXPECT_NO_THROW(spriteBatch.setRotation(1.57f)); // 90 degrees

    // Test that we can get base Sprite properties
    v2 position = spriteBatch.getPosition();
    EXPECT_EQ(position.x, 100.0f);
    EXPECT_EQ(position.y, 100.0f);

    v2 scale = spriteBatch.getScale();
    EXPECT_EQ(scale.x, 2.0f);
    EXPECT_EQ(scale.y, 2.0f);

    EXPECT_EQ(spriteBatch.getRotation(), 1.57f);
}

#include <gtest/gtest.h>
#include <cute.h>
#include "CFNativeCamera.h"
#include "../fixtures/TestFixture.hpp"

using namespace Cute;

class CFNativeCameraTest : public TestFixture
{
protected:
    std::unique_ptr<CFNativeCamera> camera;
    const float FLOAT_TOLERANCE = 0.001f;

    void SetUp() override
    {
        TestFixture::SetUp();
        camera = std::make_unique<CFNativeCamera>();
    }

    void TearDown() override
    {
        camera.reset();
        TestFixture::TearDown();
    }

    bool floatEquals(float a, float b)
    {
        return std::abs(a - b) < FLOAT_TOLERANCE;
    }

    bool v2Equals(v2 a, v2 b)
    {
        return floatEquals(a.x, b.x) && floatEquals(a.y, b.y);
    }
};

// Basic construction and properties
TEST_F(CFNativeCameraTest, DefaultConstruction)
{
    ASSERT_NE(camera, nullptr);
    EXPECT_TRUE(v2Equals(camera->getPosition(), cf_v2(0.0f, 0.0f)));
    EXPECT_TRUE(floatEquals(camera->getZoom(), 1.0f));
    EXPECT_FALSE(camera->isMoving());
}

TEST_F(CFNativeCameraTest, ParameterizedConstruction)
{
    v2 testPos = cf_v2(100.0f, 200.0f);
    float testZoom = 2.5f;

    CFNativeCamera testCamera(testPos, testZoom);

    EXPECT_TRUE(v2Equals(testCamera.getPosition(), testPos));
    EXPECT_TRUE(floatEquals(testCamera.getZoom(), testZoom));
    EXPECT_FALSE(testCamera.isMoving());
}

// Position control tests
TEST_F(CFNativeCameraTest, SetPosition)
{
    v2 newPos = cf_v2(50.0f, -30.0f);
    camera->setPosition(newPos);

    EXPECT_TRUE(v2Equals(camera->getPosition(), newPos));
}

TEST_F(CFNativeCameraTest, SetPositionComponents)
{
    float x = 75.0f, y = -125.0f;
    camera->setPosition(x, y);

    EXPECT_TRUE(v2Equals(camera->getPosition(), cf_v2(x, y)));
}

TEST_F(CFNativeCameraTest, TranslateVector)
{
    v2 startPos = cf_v2(10.0f, 20.0f);
    v2 offset = cf_v2(5.0f, -15.0f);
    v2 expectedPos = cf_v2(15.0f, 5.0f);

    camera->setPosition(startPos);
    camera->translate(offset);

    EXPECT_TRUE(v2Equals(camera->getPosition(), expectedPos));
}

TEST_F(CFNativeCameraTest, TranslateComponents)
{
    v2 startPos = cf_v2(10.0f, 20.0f);
    float dx = 5.0f, dy = -15.0f;
    v2 expectedPos = cf_v2(15.0f, 5.0f);

    camera->setPosition(startPos);
    camera->translate(dx, dy);

    EXPECT_TRUE(v2Equals(camera->getPosition(), expectedPos));
}

// Zoom control tests
TEST_F(CFNativeCameraTest, SetZoom)
{
    float testZoom = 3.0f;
    camera->setZoom(testZoom);

    EXPECT_TRUE(floatEquals(camera->getZoom(), testZoom));
}

TEST_F(CFNativeCameraTest, ZoomConstraints)
{
    camera->setZoomRange(0.5f, 4.0f);

    // Test minimum constraint
    camera->setZoom(0.1f);
    EXPECT_TRUE(floatEquals(camera->getZoom(), 0.5f));

    // Test maximum constraint
    camera->setZoom(8.0f);
    EXPECT_TRUE(floatEquals(camera->getZoom(), 4.0f));

    // Test valid range
    camera->setZoom(2.0f);
    EXPECT_TRUE(floatEquals(camera->getZoom(), 2.0f));
}

TEST_F(CFNativeCameraTest, ZoomInOut)
{
    camera->setZoom(2.0f);

    // Test zoom in
    camera->zoomIn(1.5f);
    EXPECT_TRUE(floatEquals(camera->getZoom(), 3.0f));

    // Test zoom out
    camera->zoomOut(1.5f);
    EXPECT_TRUE(floatEquals(camera->getZoom(), 2.0f));
}

// Reset functionality
TEST_F(CFNativeCameraTest, Reset)
{
    // Set some non-default values
    camera->setPosition(cf_v2(100.0f, -50.0f));
    camera->setZoom(3.5f);

    // Reset should return to defaults
    camera->reset();

    EXPECT_TRUE(v2Equals(camera->getPosition(), cf_v2(0.0f, 0.0f)));
    EXPECT_TRUE(floatEquals(camera->getZoom(), 1.0f));
}

// Target following tests
TEST_F(CFNativeCameraTest, StaticTargetFollowing)
{
    v2 targetPos = cf_v2(100.0f, 50.0f);
    camera->setTarget(targetPos);
    camera->setFollowSpeed(10.0f); // Very fast for immediate following

    // Update should move camera towards target
    camera->update(0.1f); // 100ms

    // Camera should be closer to target
    v2 currentPos = camera->getPosition();
    EXPECT_GT(currentPos.x, 0.0f);
    EXPECT_GT(currentPos.y, 0.0f);
}

TEST_F(CFNativeCameraTest, PointerTargetFollowing)
{
    v2 targetPos = cf_v2(100.0f, 50.0f);
    camera->setTarget(&targetPos);
    camera->setFollowSpeed(10.0f);

    // Update should move camera towards target
    camera->update(0.1f);

    v2 currentPos = camera->getPosition();
    EXPECT_GT(currentPos.x, 0.0f);
    EXPECT_GT(currentPos.y, 0.0f);

    // Change target position and verify camera adapts
    targetPos = cf_v2(200.0f, 100.0f);
    camera->update(0.1f);

    v2 newPos = camera->getPosition();
    EXPECT_GT(newPos.x, currentPos.x);
    EXPECT_GT(newPos.y, currentPos.y);
}

TEST_F(CFNativeCameraTest, FollowDeadzone)
{
    v2 targetPos = cf_v2(25.0f, 25.0f);
    camera->setTarget(targetPos);
    camera->setFollowDeadzone(cf_v2(30.0f, 30.0f)); // Larger deadzone than target distance
    camera->setFollowSpeed(10.0f);

    // Update - camera should not move because target is within deadzone
    camera->update(0.1f);

    EXPECT_TRUE(v2Equals(camera->getPosition(), cf_v2(0.0f, 0.0f)));
}

TEST_F(CFNativeCameraTest, ClearTarget)
{
    v2 targetPos = cf_v2(100.0f, 50.0f);
    camera->setTarget(targetPos);
    camera->clearTarget();
    camera->setFollowSpeed(10.0f);

    // Update should not move camera after clearing target
    camera->update(0.1f);

    EXPECT_TRUE(v2Equals(camera->getPosition(), cf_v2(0.0f, 0.0f)));
}

// Smooth movement tests
TEST_F(CFNativeCameraTest, SmoothMovement)
{
    v2 targetPos = cf_v2(100.0f, 50.0f);
    float duration = 1.0f; // 1 second

    camera->moveTo(targetPos, duration);

    EXPECT_TRUE(camera->isMoving());

    // After half duration, should be roughly halfway
    camera->update(0.5f);
    v2 currentPos = camera->getPosition();

    EXPECT_GT(currentPos.x, 25.0f); // Should be moving towards target
    EXPECT_LT(currentPos.x, 75.0f); // But not all the way there
    EXPECT_TRUE(camera->isMoving());

    // After full duration, should reach target
    camera->update(0.5f);
    currentPos = camera->getPosition();

    EXPECT_TRUE(v2Equals(currentPos, targetPos));
    EXPECT_FALSE(camera->isMoving());
}

TEST_F(CFNativeCameraTest, SmoothZoom)
{
    float targetZoom = 3.0f;
    float duration = 1.0f;

    camera->zoomTo(targetZoom, duration);

    EXPECT_TRUE(camera->isMoving());

    // After half duration, should be partway there
    camera->update(0.5f);
    float currentZoom = camera->getZoom();

    EXPECT_GT(currentZoom, 1.5f); // Should be zooming in
    EXPECT_LT(currentZoom, 2.5f); // But not all the way
    EXPECT_TRUE(camera->isMoving());

    // After full duration, should reach target
    camera->update(0.5f);
    currentZoom = camera->getZoom();

    EXPECT_TRUE(floatEquals(currentZoom, targetZoom));
    EXPECT_FALSE(camera->isMoving());
}

TEST_F(CFNativeCameraTest, InstantMovement)
{
    v2 targetPos = cf_v2(100.0f, 50.0f);

    // Zero duration should move instantly
    camera->moveTo(targetPos, 0.0f);

    EXPECT_TRUE(v2Equals(camera->getPosition(), targetPos));
    EXPECT_FALSE(camera->isMoving());
}

TEST_F(CFNativeCameraTest, StopMovement)
{
    v2 targetPos = cf_v2(100.0f, 50.0f);
    camera->moveTo(targetPos, 1.0f);

    EXPECT_TRUE(camera->isMoving());

    camera->stopMovement();

    EXPECT_FALSE(camera->isMoving());
}

// Camera shake tests
TEST_F(CFNativeCameraTest, CameraShake)
{
    float intensity = 10.0f;
    float duration = 0.5f;

    camera->shake(intensity, duration);

    // Update and verify shake is applied (position should change due to shake offset)
    v2 originalPos = camera->getPosition();
    camera->update(0.1f);

    // Note: This test verifies shake is active by checking it doesn't crash
    // Actual shake offset is internal and random

    // After duration expires, shake should stop
    camera->update(0.5f); // Total 0.6f seconds

    // We can't test the exact position due to random shake, but we can test
    // that the shake duration decreases over time
}

TEST_F(CFNativeCameraTest, StopShake)
{
    camera->shake(10.0f, 1.0f);
    camera->stopShake();

    // Update should not apply any shake offset
    v2 originalPos = camera->getPosition();
    camera->update(0.1f);

    EXPECT_TRUE(v2Equals(camera->getPosition(), originalPos));
}

TEST_F(CFNativeCameraTest, ShakeDecay)
{
    camera->setShakeDecay(3.0f); // Custom decay rate
    camera->shake(10.0f, 1.0f);

    // Just verify no crash with custom decay
    camera->update(0.1f);
    camera->update(0.1f);
}

// Apply/restore state tests
TEST_F(CFNativeCameraTest, ApplyRestore)
{
    // This test verifies that apply/restore don't crash
    // The actual transformation testing requires a full CF graphics context

    camera->apply();
    camera->restore();

    // Multiple applies should be safe
    camera->apply();
    camera->apply(); // Should not crash
    camera->restore();

    // Multiple restores should be safe
    camera->restore(); // Should not crash
}

// Combined functionality tests
TEST_F(CFNativeCameraTest, CombinedMovementAndFollowing)
{
    // Test smooth movement combined with target following
    v2 targetPos = cf_v2(100.0f, 50.0f);
    camera->moveTo(targetPos, 0.5f);

    // While smooth moving, set a follow target
    v2 followTarget = cf_v2(200.0f, 100.0f);
    camera->setTarget(followTarget);
    camera->setFollowSpeed(1.0f);

    // Update - both should be active
    EXPECT_TRUE(camera->isMoving());

    camera->update(0.6f); // Complete smooth movement

    // Should still be following
    camera->update(0.1f);
    v2 finalPos = camera->getPosition();

    // Should be somewhere between smooth target and follow target
    EXPECT_GT(finalPos.x, targetPos.x);
}

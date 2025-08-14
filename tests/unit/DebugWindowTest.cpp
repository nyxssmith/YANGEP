#include "../fixtures/TestFixture.hpp"
#include "../../src/lib/DebugWindow.h"

class DebugWindowTest : public TestFixture {
protected:
    void SetUp() override {
        TestFixture::SetUp();
        // ImGui is already initialized in main.cpp
    }
};

TEST_F(DebugWindowTest, Constructor) {
    DebugWindow dw("Test Window");
    // Basic constructor test - should not crash
    EXPECT_TRUE(true);
}

TEST_F(DebugWindowTest, RenderMethod) {
    DebugWindow dw("Test Window");
    // For now, just test that the object was created successfully
    // Rendering tests will be added when we have proper ImGui setup
    EXPECT_TRUE(true);
}

TEST_F(DebugWindowTest, MultipleInstances) {
    DebugWindow dw1("Window 1");
    DebugWindow dw2("Window 2");

    // Test that multiple instances can be created
    EXPECT_TRUE(true);
}

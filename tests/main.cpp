#include <gtest/gtest.h>
#include <cute.h>
#include <iostream>

// Test includes
#include "unit/DataFileTest.h"
#include "unit/DebugWindowTest.h"
#include "unit/UtilsTest.h"
#include "integration/SpriteSystemTest.h"

int main(int argc, char **argv) {
    // Initialize Cute Framework for tests that need it
    int options = CF_APP_OPTIONS_WINDOW_POS_CENTERED_BIT;
    CF_Result result = Cute::make_app("Test Window", 0, 0, 0, 640, 480, options, argv[0]);

    if (Cute::is_error(result)) {
        std::cerr << "Failed to initialize Cute Framework for tests" << std::endl;
        return -1;
    }

    // Initialize Google Test
    ::testing::InitGoogleTest(&argc, argv);

    // Run tests
    int test_result = RUN_ALL_TESTS();

    // Cleanup Cute Framework
    Cute::destroy_app();

    return test_result;
}

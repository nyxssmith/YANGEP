#include <gtest/gtest.h>
#include <cute.h>

using namespace Cute;

int main(int argc, char** argv) {
    // Initialize Cute Framework
    CF_Result result = make_app("YANGEP Tests", 0, 0, 0, 800, 600, 0, argv[0]);
    if (cf_is_error(result)) {
        printf("Failed to initialize Cute Framework: %s\n",
               result.details ? result.details : "Unknown error");
        return 1;
    }

    // Run tests
    ::testing::InitGoogleTest(&argc, argv);
    int test_result = RUN_ALL_TESTS();

    // Cleanup
    destroy_app();
    return test_result;
}

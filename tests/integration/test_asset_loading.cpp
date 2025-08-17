#include <cute.h>
#include "../../src/lib/Sprite.h"
#include <iostream>

using namespace Cute;

int main() {
    std::cout << "Asset Loading Test - Testing strict sprite loading behavior" << std::endl;
    std::cout << "=========================================================" << std::endl;

    // Test 1: Try to load a non-existent sprite
    // This should cause the program to exit with error code 1
    std::cout << "\nTest 1: Loading non-existent sprite..." << std::endl;
    std::cout << "Expected: Program should exit with 'FATAL ERROR' message" << std::endl;

    try {
        // This line should cause the program to exit immediately
        Sprite invalidSprite("tests/assets/nonexistent_sprite.png");

        // If we reach here, the strict loading failed (this shouldn't happen)
        std::cout << "ERROR: Program continued after loading invalid sprite!" << std::endl;
        std::cout << "This means the strict loading behavior is not working correctly." << std::endl;
        return 1;
    } catch (...) {
        // If an exception is thrown, that's also not the expected behavior
        std::cout << "ERROR: Exception thrown instead of program exit!" << std::endl;
        std::cout << "Expected behavior: Program should call exit(1)" << std::endl;
        return 1;
    }

    // This should never be reached if strict loading is working correctly
    std::cout << "ERROR: Reached unreachable code!" << std::endl;
    return 1;
}

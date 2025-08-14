# Testing Infrastructure Setup - Detailed Implementation Plan

## Overview
This document provides step-by-step instructions for setting up a comprehensive testing framework for YANGEP, including unit tests, integration tests, and automated testing workflows.

## Phase 1.0: Testing Infrastructure Setup

### Task 1.0.1: Set Up Testing Framework

#### Step 1: Update CMakeLists.txt
**File**: `CMakeLists.txt`

Add the following after the existing FetchContent declarations:

```cmake
# Add Google Test framework
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
    GIT_SHALLOW
)

# Add test coverage tools (optional but recommended)
option(ENABLE_COVERAGE "Enable coverage reporting" OFF)
if(ENABLE_COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

FetchContent_MakeAvailable(cute nlohmann_json googletest)

# Enable testing
enable_testing()
```

Add after the main executable:

```cmake
# Test executable
add_executable(${PROJECT_NAME}_tests
    tests/main.cpp
    tests/unit/DataFileTest.cpp
    tests/unit/DebugWindowTest.cpp
    tests/unit/UtilsTest.cpp
    tests/integration/SpriteSystemTest.cpp
    src/lib/DataFile.cpp
    src/lib/DebugWindow.cpp
    src/lib/Utils.cpp
)

# Link test executable with dependencies
target_link_libraries(${PROJECT_NAME}_tests
    cute
    nlohmann_json::nlohmann_json
    gtest
    gtest_main
)

# Include directories for tests
target_include_directories(${PROJECT_NAME}_tests
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tests>
)

# Add test to CTest
add_test(NAME ${PROJECT_NAME}_tests COMMAND ${PROJECT_NAME}_tests)

# Copy test assets
add_custom_command(TARGET ${PROJECT_NAME}_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/tests/assets
    $<TARGET_FILE_DIR:${PROJECT_NAME}_tests>/tests/assets
)
```

#### Step 2: Create Test Directory Structure
**Command**: Create the following directory structure:

```bash
mkdir -p tests/unit
mkdir -p tests/integration
mkdir -p tests/fixtures
mkdir -p tests/assets
```

#### Step 3: Create Test Main File
**File**: `tests/main.cpp`

```cpp
#include <gtest/gtest.h>
#include <cute.h>

// Test includes
#include "unit/DataFileTest.hpp"
#include "unit/DebugWindowTest.hpp"
#include "unit/UtilsTest.hpp"
#include "integration/SpriteSystemTest.hpp"

int main(int argc, char **argv) {
    // Initialize Cute Framework for tests that need it
    int options = CF_APP_OPTIONS_WINDOW_POS_CENTERED_BIT;
    CF_Result result = make_app("Test Window", 0, 0, 0, 640, 480, options, argv[0]);

    if (is_error(result)) {
        std::cerr << "Failed to initialize Cute Framework for tests" << std::endl;
        return -1;
    }

    // Initialize Google Test
    ::testing::InitGoogleTest(&argc, argv);

    // Run tests
    int test_result = RUN_ALL_TESTS();

    // Cleanup Cute Framework
    destroy_app();

    return test_result;
}
```

#### Step 4: Create Base Test Fixture
**File**: `tests/fixtures/TestFixture.hpp`

```cpp
#pragma once
#include <gtest/gtest.h>
#include <cute.h>
#include <string>

class TestFixture : public ::testing::Test {
protected:
    void SetUp() override {
        // Common setup for all tests
        mount_test_assets();
    }

    void TearDown() override {
        // Common cleanup for all tests
    }

    void mount_test_assets() {
        // Mount test assets directory
        CF_Path path = fs_get_base_directory();
        path.normalize();
        path += "/tests/assets";
        fs_mount(path.c_str(), "/test_assets");
    }

    std::string get_test_asset_path(const std::string& filename) {
        return "/test_assets/" + filename;
    }
};
```

### Task 1.0.2: Create Unit Tests for Existing Classes

#### Step 1: DataFile Unit Tests
**File**: `tests/unit/DataFileTest.cpp`

```cpp
#include "fixtures/TestFixture.hpp"
#include "DataFile.h"
#include <fstream>
#include <nlohmann/json.hpp>

class DataFileTest : public TestFixture {
protected:
    void SetUp() override {
        TestFixture::SetUp();
        create_test_json_file();
    }

    void TearDown() override {
        cleanup_test_files();
        TestFixture::TearDown();
    }

    void create_test_json_file() {
        nlohmann::json test_data = {
            {"test_key", "test_value"},
            {"number", 42},
            {"array", {1, 2, 3}}
        };

        std::ofstream file("test_data.json");
        file << test_data.dump(4);
        file.close();
    }

    void cleanup_test_files() {
        std::remove("test_data.json");
    }
};

TEST_F(DataFileTest, ConstructorWithFilename) {
    DataFile df("test_data.json");
    EXPECT_TRUE(df.contains("test_key"));
    EXPECT_EQ(df["test_key"], "test_value");
    EXPECT_EQ(df["number"], 42);
}

TEST_F(DataFileTest, LoadMethod) {
    DataFile df;
    EXPECT_TRUE(df.load("test_data.json"));
    EXPECT_TRUE(df.contains("test_key"));
}

TEST_F(DataFileTest, SaveMethod) {
    DataFile df;
    df["new_key"] = "new_value";

    EXPECT_TRUE(df.save("test_save.json"));

    // Verify saved data
    DataFile loaded_df("test_save.json");
    EXPECT_TRUE(loaded_df.contains("new_key"));
    EXPECT_EQ(loaded_df["new_key"], "new_value");

    // Cleanup
    std::remove("test_save.json");
}

TEST_F(DataFileTest, InvalidFileHandling) {
    DataFile df;
    EXPECT_FALSE(df.load("nonexistent_file.json"));
}
```

#### Step 2: DebugWindow Unit Tests
**File**: `tests/unit/DebugWindowTest.cpp`

```cpp
#include "fixtures/TestFixture.hpp"
#include "DebugWindow.h"

class DebugWindowTest : public TestFixture {
protected:
    void SetUp() override {
        TestFixture::SetUp();
        // Initialize ImGui context for testing
        cf_app_init_imgui();
    }
};

TEST_F(DebugWindowTest, Constructor) {
    DebugWindow dw("Test Window");
    // Basic constructor test - should not crash
    EXPECT_TRUE(true);
}

TEST_F(DebugWindowTest, RenderMethod) {
    DebugWindow dw("Test Window");
    // Test that render method doesn't crash
    EXPECT_NO_THROW(dw.render());
}
```

#### Step 3: Utils Unit Tests
**File**: `tests/unit/UtilsTest.cpp`

```cpp
#include "fixtures/TestFixture.hpp"
#include "Utils.h"
#include <nlohmann/json.hpp>

class UtilsTest : public TestFixture {
protected:
    void SetUp() override {
        TestFixture::SetUp();
        create_test_json_file();
    }

    void TearDown() override {
        cleanup_test_files();
        TestFixture::TearDown();
    }

    void create_test_json_file() {
        nlohmann::json test_data = {
            {"test_key", "test_value"},
            {"number", 42}
        };

        std::ofstream file("test_utils.json");
        file << test_data.dump(4);
        file.close();
    }

    void cleanup_test_files() {
        std::remove("test_utils.json");
    }
};

TEST_F(UtilsTest, ReadJsonFunction) {
    nlohmann::json result = ReadJson("test_utils.json");
    EXPECT_TRUE(result.contains("test_key"));
    EXPECT_EQ(result["test_key"], "test_value");
    EXPECT_EQ(result["number"], 42);
}

TEST_F(UtilsTest, ReadDataFileFunction) {
    DataFile result = ReadDataFile("test_utils.json");
    EXPECT_TRUE(result.contains("test_key"));
    EXPECT_EQ(result["test_key"], "test_value");
}
```

### Task 1.0.3: Create Integration Tests

#### Step 1: Sprite System Integration Test
**File**: `tests/integration/SpriteSystemTest.cpp`

```cpp
#include "fixtures/TestFixture.hpp"
#include "DataFile.h"
#include "Utils.h"

class SpriteSystemIntegrationTest : public TestFixture {
protected:
    void SetUp() override {
        TestFixture::SetUp();
        create_test_assets();
    }

    void TearDown() override {
        cleanup_test_assets();
        TestFixture::TearDown();
    }

    void create_test_assets() {
        // Create test JSON configuration
        nlohmann::json sprite_config = {
            {"sprites", {
                {
                    {"name", "test_sprite"},
                    {"texture", "test_texture.png"},
                    {"position", {100, 100}},
                    {"scale", {1.0, 1.0}}
                }
            }}
        };

        std::ofstream file("test_sprite_config.json");
        file << sprite_config.dump(4);
        file.close();
    }

    void cleanup_test_assets() {
        std::remove("test_sprite_config.json");
    }
};

TEST_F(SpriteSystemIntegrationTest, LoadSpriteConfiguration) {
    // Test loading sprite configuration from JSON
    DataFile config("test_sprite_config.json");
    EXPECT_TRUE(config.contains("sprites"));

    auto sprites = config["sprites"];
    EXPECT_EQ(sprites.size(), 1);

    auto sprite = sprites[0];
    EXPECT_EQ(sprite["name"], "test_sprite");
    EXPECT_EQ(sprite["texture"], "test_texture.png");
}

TEST_F(SpriteSystemIntegrationTest, EndToEndSpriteLoading) {
    // Test complete sprite loading workflow
    nlohmann::json result = ReadJson("test_sprite_config.json");
    EXPECT_TRUE(result.contains("sprites"));

    // This test will be expanded when Sprite class is implemented
    EXPECT_TRUE(true);
}
```

### Task 1.0.4: Create Test Assets

#### Step 1: Test JSON Files
**File**: `tests/assets/test_character.json`

```json
{
    "character": {
        "name": "test_skeleton",
        "components": {
            "head": {
                "texture": "HEAD_chain_armor_helmet.png",
                "offset": [0, 0],
                "layer": 1
            },
            "body": {
                "texture": "BODY_skeleton.png",
                "offset": [0, 0],
                "layer": 0
            }
        },
        "animations": {
            "idle": {
                "frames": 1,
                "duration": 1.0
            },
            "walk": {
                "frames": 4,
                "duration": 0.8
            }
        }
    }
}
```

#### Step 2: Test Texture Atlas Configuration
**File**: `tests/assets/test_atlas.json`

```json
{
    "atlas": {
        "texture": "test_atlas.png",
        "regions": {
            "head": {
                "x": 0,
                "y": 0,
                "width": 32,
                "height": 32
            },
            "body": {
                "x": 32,
                "y": 0,
                "width": 64,
                "height": 64
            }
        }
    }
}
```

### Task 1.0.5: Update Build Scripts

#### Step 1: Update makefile
**File**: `makefile`

Add the following target:

```makefile
test: build
	@echo "Running tests..."
	./scripts/test.sh

test-coverage: build
	@echo "Running tests with coverage..."
	./scripts/test.sh --coverage
```

#### Step 2: Create Test Script
**File**: `scripts/test.sh`

```bash
#!/bin/bash
source "$(dirname "$0")/common.sh"

cd "$BUILD_DIR"

# Check if coverage is requested
COVERAGE=false
if [[ "$1" == "--coverage" ]]; then
    COVERAGE=true
    echo "Enabling coverage reporting..."
fi

# Build tests if needed
if [[ ! -f "yangep_tests" ]]; then
    echo "Building tests..."
    make
fi

# Run tests
echo "Running tests..."
if [[ "$COVERAGE" == true ]]; then
    # Run with coverage
    ./yangep_tests --gtest_output=xml:test_results.xml

    # Generate coverage report (if lcov is available)
    if command -v lcov &> /dev/null; then
        lcov --capture --directory . --output-file coverage.info
        lcov --remove coverage.info '/usr/*' --output-file coverage.info
        lcov --list coverage.info
    fi
else
    # Run without coverage
    ./yangep_tests
fi

# Check test exit code
TEST_EXIT_CODE=$?
if [[ $TEST_EXIT_CODE -eq 0 ]]; then
    echo "✅ All tests passed!"
else
    echo "❌ Some tests failed!"
    exit $TEST_EXIT_CODE
fi
```

#### Step 3: Make Test Script Executable
**Command**:
```bash
chmod +x scripts/test.sh
```

## Testing Workflow

### Daily Testing
1. **Before committing**: Run `make test` to ensure all tests pass
2. **After major changes**: Run `make test-coverage` to check coverage
3. **Before PR**: Ensure integration tests pass

### Test Development Workflow
1. **Write test first** (TDD approach)
2. **Implement feature** to make test pass
3. **Refactor** while keeping tests green
4. **Add more tests** for edge cases

### Continuous Integration
- Tests should run automatically on every build
- Coverage reports should be generated
- Failed tests should block deployment

## Success Criteria

### Phase 1.0 Success
- [ ] `make test` command works and runs all tests
- [ ] Test framework loads without errors
- [ ] All existing class tests pass
- [ ] Test coverage reporting works
- [ ] Integration tests can access project dependencies
- [ ] Test assets load correctly

### Quality Metrics
- **Test Coverage**: >90% for new code
- **Test Execution Time**: <30 seconds for full test suite
- **Test Reliability**: 100% pass rate on clean builds
- **Documentation**: All test classes documented

## Next Steps After Testing Setup

Once the testing infrastructure is complete:

1. **Begin Task 1.1.1**: Create Basic Sprite Class Structure
2. **Write tests first** for each new class
3. **Implement features** to make tests pass
4. **Maintain test coverage** throughout development

## Troubleshooting

### Common Issues
1. **Cute Framework not available in tests**
   - Ensure proper initialization in test main
   - Check that tests run in correct environment

2. **Asset loading failures**
   - Verify test assets are copied to build directory
   - Check file paths and permissions

3. **Memory leaks in tests**
   - Use valgrind or similar tools
   - Ensure proper cleanup in TearDown methods

### Debug Commands
```bash
# Run specific test
./yangep_tests --gtest_filter=DataFileTest.*

# Run with verbose output
./yangep_tests --gtest_verbose

# Run with coverage
make test-coverage
```

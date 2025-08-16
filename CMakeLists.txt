cmake_minimum_required(VERSION 3.28...3.30)

project(
	yangep
	VERSION 0.0.1
	LANGUAGES CXX
)

# Must have at least C++20.
set(CMAKE_CXX_STANDARD 20)

# Set default build type to Debug if not specified
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug)
endif()

# Add debug flags for Debug builds
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
	set(CMAKE_C_FLAGS_DEBUG "-g -O0 -DDEBUG")
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Emscripten")
	set(EMSCRIPTEN TRUE)
endif()

# Make sure all binaries are placed into the same build folder.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Disable building CF samples by default
set(CF_FRAMEWORK_BUILD_SAMPLES OFF)

# Disable building CF tests by default
set(CF_FRAMEWORK_BUILD_TESTS OFF)

# This will download and build Cute Framework just once the first time you build your game.
include(FetchContent)
FetchContent_Declare(
	cute
	GIT_REPOSITORY https://github.com/RandyGaul/cute_framework
	GIT_TAG master
	GIT_SHALLOW
)

# Add nlohmann/json library
FetchContent_Declare(
	nlohmann_json
	GIT_REPOSITORY https://github.com/nlohmann/json.git
	GIT_TAG v3.11.3
	GIT_SHALLOW
)

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

# Source code for your game.
# YANGEP executable
add_executable(yangep
    src/main.cpp
    src/lib/SpriteBatch.cpp
    src/lib/SpriteBatchDemo.cpp
    src/lib/Sprite.cpp
    src/lib/DataFile.cpp
    src/lib/Utils.cpp
)

# Our source code will be in the `src` folder.
target_include_directories(yangep PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)

# Some basic information needed for CMake to generate your Info.plist file.
# This is necessary for e.g. iOS builds.
if(APPLE)
    set_target_properties(
        yangep
        PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.myteam.yangep"
        MACOSX_BUNDLE_BUNDLE_VERSION "1.0.0"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0.0"
    )
endif()

# Make the game link against Cute Framework and nlohmann/json.
target_link_libraries(yangep cute nlohmann_json::nlohmann_json)

# Test executable
add_executable(
    yangep_tests
    tests/main.cpp
    tests/unit/DataFileTest.cpp
    tests/unit/UtilsTest.cpp
    tests/unit/SpriteTest.cpp
    tests/integration/SpriteSystemTest.cpp
    tests/integration/AssetLoadingTest.cpp
    tests/unit/SpriteBatchTest.cpp
    src/lib/DataFile.cpp
    src/lib/Utils.cpp
    src/lib/Sprite.cpp
    src/lib/SpriteBatch.cpp
    src/lib/SpriteBatchDemo.cpp
)

# Link test executable with dependencies
target_link_libraries(yangep_tests
    cute
    nlohmann_json::nlohmann_json
    gtest
    gtest_main
)

# Include directories for tests
target_include_directories(yangep_tests
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tests>
)

# Add test to CTest
add_test(NAME yangep_tests COMMAND yangep_tests)

# Asset loading test executable (separate from main tests)
add_executable(yangep_asset_loading_test
    tests/integration/test_asset_loading.cpp
    src/lib/Sprite.cpp
)

# Link asset loading test with dependencies
target_link_libraries(yangep_asset_loading_test
    cute
    nlohmann_json::nlohmann_json
)

# Include directories for asset loading test
target_include_directories(yangep_asset_loading_test
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

# Copy test assets
add_custom_command(TARGET yangep_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/tests/assets
    $<TARGET_FILE_DIR:yangep_tests>/tests/assets
)

# For convenience on Windows, set MSVC debugger's working directory in the build folder.
# Also ask MSVC to make the game the startup project.
if (MSVC)
    set_property(TARGET yangep PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:yangep>)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT yangep)
endif()
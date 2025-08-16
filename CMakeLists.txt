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
# Main executable
add_executable(yangep
    src/main.cpp
    src/lib/PNGSprite.cpp
    src/lib/PNGSpriteDemo.cpp
    src/lib/SpriteSheetSplitter.cpp
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

# For convenience on Windows, set MSVC debugger's working directory in the build folder.
# Also ask MSVC to make the game the startup project.
if (MSVC)
    set_property(TARGET yangep PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:yangep>)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT yangep)
endif()

# Runtime test executable (simple test to show CF_ASSERT failure)
add_executable(yangep_runtime_test
    src/lib/PNGSpriteRuntimeTest.cpp
)

target_link_libraries(yangep_runtime_test cute)

# Test executable
add_executable(yangep_tests
    tests/main.cpp
    tests/unit/PNGSpriteTest.cpp
    tests/integration/SpriteSystemIntegrationTest.cpp
    src/lib/PNGSprite.cpp
    src/lib/SpriteSheetSplitter.cpp
)

target_link_libraries(yangep_tests gtest gtest_main cute)
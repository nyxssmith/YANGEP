cmake_minimum_required(VERSION 3.28...3.30)

project(
	yangep
	VERSION 0.0.1
	LANGUAGES CXX
)

# Export compile commands for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
	GIT_TAG 0124e35 # master as of Oct 25 2025
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

# Add pugixml library
FetchContent_Declare(
	pugixml
	GIT_REPOSITORY https://github.com/zeux/pugixml.git
	GIT_TAG v1.14
	GIT_SHALLOW
)

# Add libspng library
FetchContent_Declare(
	libspng
	GIT_REPOSITORY https://github.com/randy408/libspng.git
	GIT_TAG v0.7.4
	GIT_SHALLOW
)

# Add test coverage tools (optional but recommended)
option(ENABLE_COVERAGE "Enable coverage reporting" OFF)
if(ENABLE_COVERAGE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

FetchContent_MakeAvailable(cute nlohmann_json googletest pugixml libspng)

# Enable testing
enable_testing()

# Source code for your game.
add_executable(
	${PROJECT_NAME}
	src/main.cpp
	src/lib/DataFile.cpp
	src/lib/RealConfigFile.cpp
	src/lib/DebugWindow.cpp
	src/lib/DataFileDebugWindow.cpp
	src/lib/DebugWindowList.cpp
	src/lib/DebugFPSWindow.cpp
	src/lib/DebugJobWindow.cpp
	src/lib/Utils.cpp
	src/lib/tsx.cpp
	src/lib/tmx.cpp
	src/lib/CFNativeCamera.cpp
	src/lib/SpriteAnimationLoader.cpp
	src/lib/NavMesh.cpp
	src/lib/NavMeshPoint.cpp
	src/lib/NavMeshPath.cpp
	src/lib/AnimatedDataCharacter.cpp
	src/lib/AnimatedDataCharacterNavMeshAgent.cpp
	src/lib/Behavior.cpp
	src/lib/WanderBehavior.cpp
	src/lib/LevelV1.cpp
	src/lib/JobSystem.cpp
)

# Our source code will be in the `src` folder.
target_include_directories(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)

# Some basic information needed for CMake to generate your Info.plist file.
# This is necessary for e.g. iOS builds.
if(APPLE)
    set_target_properties(
        ${PROJECT_NAME}
        PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.myteam.yangep"
        MACOSX_BUNDLE_BUNDLE_VERSION "1.0.0"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0.0"
    )
endif()

# Make the game link against Cute Framework and other dependencies
target_link_libraries(${PROJECT_NAME}
    cute
    nlohmann_json::nlohmann_json
    pugixml
    spng_static
)

# For convenience on Windows, set MSVC debugger's working directory in the build folder.
# Also ask MSVC to make the game the startup project.
if (MSVC)
    set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:${PROJECT_NAME}>)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

    # Also set working directory for animation demo
    set_property(TARGET ${PROJECT_NAME}_animation_demo PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:${PROJECT_NAME}_animation_demo>)
endif()





# Test executable
add_executable(${PROJECT_NAME}_tests
    tests/main.cpp
    tests/unit/SpriteAnimationLoaderTest.cpp
    tests/unit/TMXTest.cpp
    tests/unit/PNGValidationTest.cpp
    tests/unit/CFNativeCameraTest.cpp
    tests/integration/SpriteSystemIntegrationTest.cpp
    tests/integration/TMXRenderingTest.cpp
    src/lib/SpriteAnimationLoader.cpp
    src/lib/CFNativeCamera.cpp
    src/lib/DataFile.cpp
    src/lib/Utils.cpp
    src/lib/tsx.cpp
    src/lib/tmx.cpp
    src/lib/NavMesh.cpp
    src/lib/NavMeshPoint.cpp
    src/lib/NavMeshPath.cpp
    src/lib/AnimatedDataCharacter.cpp
    src/lib/AnimatedDataCharacterNavMeshAgent.cpp
    src/lib/Behavior.cpp
    src/lib/WanderBehavior.cpp
    src/lib/LevelV1.cpp
    src/lib/JobSystem.cpp
)

target_link_libraries(${PROJECT_NAME}_tests gtest gtest_main cute spng_static nlohmann_json::nlohmann_json pugixml-static)
target_include_directories(${PROJECT_NAME}_tests PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)
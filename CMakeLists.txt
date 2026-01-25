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

# Make AppleClang treat c++11 narrowing as a warning instead of an error
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	add_compile_options(-Wno-error=c++11-narrowing)
endif()

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
	GIT_TAG v1.15
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
	src/lib/FileHandling/DataFile.cpp
	src/lib/FileHandling/RealConfigFile.cpp
	src/lib/FileHandling/Utils.cpp
	src/lib/Debug/DebugWindow.cpp
	src/lib/Debug/DataFileDebugWindow.cpp
	src/lib/Debug/DebugWindowList.cpp
	src/lib/Debug/DebugFPSWindow.cpp
	src/lib/Debug/DebugJobWindow.cpp
	src/lib/Debug/DebugPlayerInfoWindow.cpp
	src/lib/Debug/DebugCharacterInfoWindow.cpp
	src/lib/Debug/DebugCoordinatorWindow.cpp
	src/lib/Debug/DebugStateMachineWindow.cpp
	src/lib/Debug/DebugStateWindow.cpp
	src/lib/Level/FileHandling/tsx.cpp
	src/lib/Level/FileHandling/tmx.cpp
	src/lib/Level/GameLogic/LevelMap.cpp
	src/lib/Level/GameLogic/LevelV1.cpp
	src/lib/Level/GameLogic/WorldPositionRenderedObjectsList.cpp
	src/lib/Level/GameLogic/NavMesh.cpp
	src/lib/Level/GameLogic/NavMeshPoint.cpp
	src/lib/Level/GameLogic/NavMeshPath.cpp
	src/lib/Level/GameLogic/SpatialGrid.cpp
	src/lib/Camera/CFNativeCamera.cpp
	src/lib/Camera/Camera.cpp
	src/lib/Character/FileHandling/SpriteAnimationLoader.cpp
	src/lib/Character/AnimatedDataCharacter.cpp
	src/lib/Character/AnimatedDataCharacterNavMeshAgent.cpp
	src/lib/Character/AnimatedDataCharacterNavMeshPlayer.cpp
	src/lib/Character/Coordinator/Coordinator.cpp
	src/lib/Character/Coordinator/NearPlayerTileGrid.cpp
	src/lib/Character/StateMachines/State.cpp
	src/lib/Character/StateMachines/StateMachine.cpp
	src/lib/Character/StateMachines/StateMachineController.cpp
	src/lib/Character/StateMachines/StateLibrary.cpp
	src/lib/Character/StateMachines/States/WaitState.cpp
	src/lib/Character/StateMachines/States/PrintState.cpp
	src/lib/Character/StateMachines/States/WanderNewPositionState.cpp
	src/lib/Character/StateMachines/States/MoveToPositionState.cpp
	src/lib/Combat/HitBox.cpp
	src/lib/Combat/Action.cpp
	src/lib/Combat/Damage.cpp
	src/lib/Combat/ABActions.cpp
	src/lib/JobSystem/JobSystem.cpp
	src/lib/JobSystem/OnScreenChecks.cpp
	src/lib/Effects/RedFlashEffect.cpp
	src/lib/Effects/GreenFlashEffect.cpp
	src/lib/Effects/EffectFactory.cpp
	src/lib/Effects/ShaderRegistry.cpp
	src/lib/Effects/TrailGhostEffect.cpp
	src/lib/Effects/DissolveEffect.cpp
	src/lib/Effects/GhostTrailRenderer.cpp
	src/lib/UI/HighlightTile.cpp
	src/lib/UI/ColorUtils.cpp
	src/lib/Items/Item.cpp
	src/lib/Items/Inventory.cpp
)

# Our source code will be in the `src` folder.
target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lib/Camera>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lib/Character>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lib/Character/Behavior>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lib/Character/Coordinator>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lib/Character/FileHandling>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lib/Character/StateMachines>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lib/Combat>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lib/Debug>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lib/FileHandling>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lib/Effects>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lib/JobSystem>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lib/Level/FileHandling>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lib/Level/GameLogic>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lib/UI>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lib/Items>
)

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
endif()

# Action Editor executable
add_executable(
	${PROJECT_NAME}_action_editor
	actionEditor/main.cpp
	src/lib/FileHandling/DataFile.cpp
	src/lib/FileHandling/RealConfigFile.cpp
	src/lib/FileHandling/Utils.cpp
	src/lib/Debug/DebugWindow.cpp
	src/lib/Debug/DataFileDebugWindow.cpp
	src/lib/Debug/DebugWindowList.cpp
	src/lib/Debug/DebugFPSWindow.cpp
	src/lib/Debug/DebugJobWindow.cpp
	src/lib/Debug/DebugPlayerInfoWindow.cpp
	src/lib/Debug/DebugCharacterInfoWindow.cpp
	src/lib/Debug/DebugCoordinatorWindow.cpp
	src/lib/Debug/DebugStateMachineWindow.cpp
	src/lib/Debug/DebugStateWindow.cpp
	src/lib/Level/FileHandling/tsx.cpp
	src/lib/Level/FileHandling/tmx.cpp
	src/lib/Level/GameLogic/LevelMap.cpp
	src/lib/Level/GameLogic/LevelV1.cpp
	src/lib/Level/GameLogic/WorldPositionRenderedObjectsList.cpp
	src/lib/Level/GameLogic/NavMesh.cpp
	src/lib/Level/GameLogic/NavMeshPoint.cpp
	src/lib/Level/GameLogic/NavMeshPath.cpp
	src/lib/Level/GameLogic/SpatialGrid.cpp
	src/lib/Camera/CFNativeCamera.cpp
	src/lib/Camera/Camera.cpp
	src/lib/Character/FileHandling/SpriteAnimationLoader.cpp
	src/lib/Character/AnimatedDataCharacter.cpp
	src/lib/Character/AnimatedDataCharacterNavMeshAgent.cpp
	src/lib/Character/AnimatedDataCharacterNavMeshPlayer.cpp
	src/lib/Character/Coordinator/Coordinator.cpp
	src/lib/Character/Coordinator/NearPlayerTileGrid.cpp
	src/lib/Character/StateMachines/State.cpp
	src/lib/Character/StateMachines/StateMachine.cpp
	src/lib/Character/StateMachines/StateMachineController.cpp
	src/lib/Character/StateMachines/StateLibrary.cpp
	src/lib/Character/StateMachines/States/WaitState.cpp
	src/lib/Character/StateMachines/States/PrintState.cpp
	src/lib/Character/StateMachines/States/WanderNewPositionState.cpp
	src/lib/Character/StateMachines/States/MoveToPositionState.cpp
	src/lib/Combat/HitBox.cpp
	src/lib/Combat/Action.cpp
	src/lib/Combat/Damage.cpp
	src/lib/Combat/ABActions.cpp
	src/lib/JobSystem/JobSystem.cpp
	src/lib/JobSystem/OnScreenChecks.cpp
	src/lib/Effects/RedFlashEffect.cpp
	src/lib/Effects/GreenFlashEffect.cpp
	src/lib/Effects/EffectFactory.cpp
	src/lib/Effects/ShaderRegistry.cpp
	src/lib/Effects/TrailGhostEffect.cpp
	src/lib/Effects/DissolveEffect.cpp
	src/lib/Effects/GhostTrailRenderer.cpp
	src/lib/UI/HighlightTile.cpp
	src/lib/UI/ColorUtils.cpp
	src/lib/Items/Item.cpp
	src/lib/Items/Inventory.cpp
)

target_include_directories(${PROJECT_NAME}_action_editor PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lib/Camera>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lib/Character>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lib/Character/Behavior>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lib/Character/Coordinator>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lib/Character/FileHandling>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lib/Character/StateMachines>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lib/Combat>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lib/Debug>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lib/FileHandling>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lib/Effects>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lib/JobSystem>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lib/Level/FileHandling>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lib/Level/GameLogic>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lib/UI>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lib/Items>
)

target_link_libraries(${PROJECT_NAME}_action_editor
	cute
	nlohmann_json::nlohmann_json
	pugixml
	spng_static
)

if (MSVC)
	set_property(TARGET ${PROJECT_NAME}_action_editor PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:${PROJECT_NAME}_action_editor>)
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
    src/lib/Character/FileHandling/SpriteAnimationLoader.cpp
    src/lib/Camera/CFNativeCamera.cpp
    src/lib/FileHandling/DataFile.cpp
    src/lib/FileHandling/Utils.cpp
    src/lib/Level/FileHandling/tsx.cpp
    src/lib/Level/FileHandling/tmx.cpp
    src/lib/Level/GameLogic/LevelMap.cpp
    src/lib/Level/GameLogic/NavMesh.cpp
    src/lib/Level/GameLogic/NavMeshPoint.cpp
    src/lib/Level/GameLogic/NavMeshPath.cpp
    src/lib/Level/GameLogic/SpatialGrid.cpp
    src/lib/Level/GameLogic/LevelV1.cpp
    src/lib/Level/GameLogic/WorldPositionRenderedObjectsList.cpp
    src/lib/Character/AnimatedDataCharacter.cpp
    src/lib/Combat/HitBox.cpp
    src/lib/Combat/Action.cpp
    src/lib/Combat/Damage.cpp
    src/lib/Combat/ABActions.cpp
    src/lib/Character/AnimatedDataCharacterNavMeshAgent.cpp
    src/lib/Character/AnimatedDataCharacterNavMeshPlayer.cpp
    src/lib/Character/Coordinator/Coordinator.cpp
    src/lib/Character/Coordinator/NearPlayerTileGrid.cpp
    src/lib/Character/StateMachines/State.cpp
    src/lib/Character/StateMachines/StateMachine.cpp
    src/lib/Character/StateMachines/StateMachineController.cpp
    src/lib/Character/StateMachines/StateLibrary.cpp
    src/lib/Character/StateMachines/States/WaitState.cpp
    src/lib/Character/StateMachines/States/PrintState.cpp
    src/lib/Character/StateMachines/States/WanderNewPositionState.cpp
    src/lib/Character/StateMachines/States/MoveToPositionState.cpp
    src/lib/JobSystem/JobSystem.cpp
    src/lib/JobSystem/OnScreenChecks.cpp
	src/lib/Effects/RedFlashEffect.cpp
    src/lib/Effects/GreenFlashEffect.cpp
    src/lib/Effects/EffectFactory.cpp
    src/lib/Effects/ShaderRegistry.cpp
	src/lib/Effects/TrailGhostEffect.cpp
	src/lib/Effects/DissolveEffect.cpp
	src/lib/Effects/GhostTrailRenderer.cpp
    src/lib/UI/HighlightTile.cpp
    src/lib/UI/ColorUtils.cpp
    src/lib/Items/Item.cpp
    src/lib/Items/Inventory.cpp
)

target_link_libraries(${PROJECT_NAME}_tests gtest gtest_main cute spng_static nlohmann_json::nlohmann_json pugixml-static)
target_include_directories(${PROJECT_NAME}_tests PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lib/Camera>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lib/Character>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lib/Character/Behavior>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lib/Character/Coordinator>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lib/Character/FileHandling>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lib/Character/StateMachines>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lib/Combat>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lib/Debug>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lib/FileHandling>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lib/Effects>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lib/JobSystem>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lib/Level/FileHandling>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lib/Level/GameLogic>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lib/UI>
)
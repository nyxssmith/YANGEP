#include "SceneManager.h"
#include <cute.h>
#include <stdio.h>
#include <cstdlib>
#include <memory>
#include <fstream>
#include <dcimgui.h>
#include "DebugWindow.h"
#include "DataFileDebugWindow.h"
#include "DebugWindowList.h"
#include "DebugFPSWindow.h"
#include "DebugJobWindow.h"
#include "DebugPlayerInfoWindow.h"
#include "DebugCharacterInfoWindow.h"
#include "DebugCoordinatorWindow.h"
#include "DebugInputInfoWindow.h"
#include "DebugSceneSwitcherWindow.h"
#include "OnScreenChecks.h"
#include "Coordinator.h"
#include "Utils.h"
#include "DataFile.h"
#include "RealConfigFile.h"
#include "LevelV1.h"
#include "JobSystem.h"

#include "CFNativeCamera.h"
#include "NavMesh.h"
#include "NavMeshPath.h"
#include "AnimatedDataCharacterNavMeshPlayer.h"
#include "ShaderRegistry.h"
#include "HighlightTile.h"
#include "AtlasLabelerWindow.h"
#include "HudUI.h"
#include "Inventory.h"
#include "Item.h"
#include "SceneManager.h"

SceneManager::SceneManager() = default;

SceneManager::~SceneManager()
{
}

bool SceneManager::LoadScene(const std::string &scene_name)
{
    // cleans up current scene if exists, then force loads the input one
    if (currentScene)
    {
        Cleanup();
    }
    // TODO add toggle to re-run setup or not, if not it lets windowconfig changes live in memory!
    //  re-run setup to refresh jsons
    // SharedSetup(viewportWidth, viewportHeight);
    //  load new scene into current scene pointer
    currentScene = std::make_unique<Scene>();
    if (!currentScene->init(scene_name))
    {
        currentScene = nullptr;
        printf("Failed to initialize scene: %s\n", scene_name.c_str());
        return false;
    }
    // Share the single camera instance so it survives across scene swaps
    currentScene->setCfCamera(cfCamera);
    currentScene->setWindowConfig(windowConfig);
    currentScene->setViewportWidth(viewportWidth);
    currentScene->setViewportHeight(viewportHeight);
    currentScene->setViewportZoom(viewportZoom);
    currentScene->setShowNavMesh(debugHighlightNavmesh);
    currentScene->setShowNavMeshPoints(debughighlightNavMeshPaths);
    currentScene->setShowAgents(debugHighlightAgents);
    currentScene->setDebugWindows(debugWindows);
    currentScene->setAtlasLabeler(atlasLabeler);
    currentScene->setAtlasLabelerOpen(atlas_labeler_open);
    currentScene->setShowFPSMetrics(ShowFPSMetrics);
    currentScene->setShowJobMetrics(ShowJobMetrics);
    currentScene->setShowPlayerInfo(ShowPlayerInfo);
    currentScene->setShowInputInfo(ShowInputInfo);
    currentScene->setShowCoordinatorInfo(ShowCoordinatorInfo);
    currentScene->setDebugSceneSwitcherWindow(debugSceneSwitcherWindow.get());
    currentScene->setCharacterInfoWindows(characterInfoWindows);
    currentScene->setRecordInputInfo(recordInputInfo);
    currentScene->setInputLogFile(inputLogFile);
    return true;
}

Scene *SceneManager::GetcurrentScene()
{
    return currentScene.get();
}

void SceneManager::SharedSetup(int windowWidth, int windowHeight)
{
    // Set shader directory for runtime-compiled draw shaders
    cf_shader_directory("/assets/shaders");
    // Register and compile shaders at boot
    ShaderRegistry::registerAndLoadAll();

    // Load window configuration again using VFS for viewport and debug windows
    windowConfig = DataFile("/assets/window-config.json");

    // Read viewport dimensions from config (defaults to window size)
    viewportWidth = (float)windowWidth;
    viewportHeight = (float)windowHeight;
    viewportZoom = 1.0f;            // Default zoom level
    debugHighlightViewport = false; // Default: don't highlight viewport

    if (windowConfig.contains("window"))
    {
        auto &window = windowConfig["window"];

        // Check for viewportScale first - if present, multiply window dimensions
        if (window.contains("viewportScale"))
        {
            float viewportScale = window["viewportScale"];
            viewportWidth = (float)windowWidth * viewportScale;
            viewportHeight = (float)windowHeight * viewportScale;
            printf("Using viewport scale %.2f: viewport=%.0fx%.0f (window=%dx%d)\n",
                   viewportScale, viewportWidth, viewportHeight, windowWidth, windowHeight);
        }
        // Otherwise, check for explicit viewport dimensions
        else if (window.contains("viewportWidth") && window.contains("viewportHeight"))
        {
            viewportWidth = window["viewportWidth"];
            viewportHeight = window["viewportHeight"];
            printf("Loaded viewport config: %.0fx%.0f\n", viewportWidth, viewportHeight);
        }
        else
        {
            printf("No viewport size in config, using window size: %.0fx%.0f\n", viewportWidth, viewportHeight);
        }

        if (window.contains("viewportZoom"))
        {
            viewportZoom = window["viewportZoom"];
            printf("Loaded viewport zoom: %.2f\n", viewportZoom);
        }
    }

    // Create the single shared camera instance, sized for this viewport/zoom.
    // Scenes point at this instance (via setCfCamera) so it survives scene swaps.
    cfCamera = CFNativeCamera(cf_v2(0.0f, 0.0f), viewportZoom, viewportWidth, viewportHeight);

    // Read debug options from config
    debugHighlightNavmesh = false;                   // Default: don't highlight navmesh
    debughighlightNavMeshPaths = false;              // Default: don't highlight navmesh points
    debugHighlightAgents = false;                    // Default: don't highlight agents
    debugHighlightCharacterHitboxes = false;         // Default: don't show character hitboxes
    debugHighlightSpatialGrid = false;               // Default: don't show spatial grid
    debugHighlightCoordinatorInfo = false;           // Default: don't show coordinator info
    debugHighlightPlayerNavmeshCollisionBox = false; // Default: don't show player navmesh collision box
    clickToInspectCharacter = false;                 // Default: don't inspect character on click
    debugEntityScale = 1.0f;
    if (windowConfig.contains("Debug"))
    {
        auto &debug = windowConfig["Debug"];
        if (debug.contains("highlightViewport"))
        {
            debugHighlightViewport = debug["highlightViewport"];
            printf("Debug highlightViewport: %s\n", debugHighlightViewport ? "enabled" : "disabled");
        }
        if (debug.contains("highlightNavmesh"))
        {
            debugHighlightNavmesh = debug["highlightNavmesh"];
            printf("Debug highlightNavmesh: %s\n", debugHighlightNavmesh ? "enabled" : "disabled");
        }
        if (debug.contains("highlightNavMeshPaths"))
        {
            debughighlightNavMeshPaths = debug["highlightNavMeshPaths"];
            printf("Debug highlightNavMeshPaths: %s\n", debughighlightNavMeshPaths ? "enabled" : "disabled");
        }
        if (debug.contains("highlightAgents"))
        {
            debugHighlightAgents = debug["highlightAgents"];
            printf("Debug highlightAgents: %s\n", debugHighlightAgents ? "enabled" : "disabled");
        }
        if (debug.contains("highlightCharacterHitboxes"))
        {
            debugHighlightCharacterHitboxes = debug["highlightCharacterHitboxes"];
            printf("Debug highlightCharacterHitboxes: %s\n", debugHighlightCharacterHitboxes ? "enabled" : "disabled");
        }
        if (debug.contains("highlightSpatialGrid"))
        {
            debugHighlightSpatialGrid = debug["highlightSpatialGrid"];
            printf("Debug highlightSpatialGrid: %s\n", debugHighlightSpatialGrid ? "enabled" : "disabled");
        }
        if (debug.contains("highlightCoordinatorInfo"))
        {
            debugHighlightCoordinatorInfo = debug["highlightCoordinatorInfo"];
            printf("Debug highlightCoordinatorInfo: %s\n", debugHighlightCoordinatorInfo ? "enabled" : "disabled");
        }
        if (debug.contains("highlightPlayerNavmeshCollisionBox"))
        {
            debugHighlightPlayerNavmeshCollisionBox = debug["highlightPlayerNavmeshCollisionBox"];
            printf("Debug highlightPlayerNavmeshCollisionBox: %s\n", debugHighlightPlayerNavmeshCollisionBox ? "enabled" : "disabled");
        }
        if (debug.contains("EntityScale") && debug["EntityScale"].is_number())
        {
            debugEntityScale = debug["EntityScale"];
            printf("Debug EntityScale: %.2f\n", debugEntityScale);
        }
        if (debug.contains("clickToInspectCharacter"))
        {
            clickToInspectCharacter = debug["clickToInspectCharacter"];
            printf("Debug clickToInspectCharacter: %s\n", clickToInspectCharacter ? "enabled" : "disabled");
        }
    }

    // Create debug window list and populate from config
    // DebugWindowList debugWindows;
    // AtlasLabelerWindow atlasLabeler; // simple always-on tool window
    atlas_labeler_open = false;

    // Load debug windows from config
    printf("Checking for DebugWindows in config...\n");
    printf("Config contains DebugWindows: %s\n", windowConfig.contains("DebugWindows") ? "yes" : "no");

    if (windowConfig.contains("DebugWindows"))
    {
        printf("DebugWindows is_array: %s\n", windowConfig["DebugWindows"].is_array() ? "yes" : "no");
    }

    if (windowConfig.contains("DebugWindows") && windowConfig["DebugWindows"].is_array())
    {
        printf("Number of entries in DebugWindows array: %zu\n", windowConfig["DebugWindows"].size());

        for (const auto &debugWindowEntry : windowConfig["DebugWindows"])
        {
            printf("Processing debug window entry...\n");
            printf("  Contains 'enabled': %s\n", debugWindowEntry.contains("enabled") ? "yes" : "no");

            if (debugWindowEntry.contains("enabled"))
            {
                bool enabled = debugWindowEntry["enabled"].get<bool>();
                printf("  Enabled: %s\n", enabled ? "yes" : "no");
            }

            if (debugWindowEntry.contains("enabled") && debugWindowEntry["enabled"].get<bool>())
            {
                if (debugWindowEntry.contains("dataFilePath"))
                {
                    std::string path = debugWindowEntry["dataFilePath"];
                    printf("  Loading debug window for: %s\n", path.c_str());
                    debugWindows.add(path);
                }
                else
                {
                    printf("  No dataFilePath found\n");
                }
            }
        }
    }
    else
    {
        printf("DebugWindows not found or not an array\n");
    }

    printf("Loaded %zu debug windows from config\n", debugWindows.count());

    // Create FPS metrics debug window if enabled in config
    std::unique_ptr<DebugFPSWindow> fpsWindow;
    ShowFPSMetrics = false;

    // Create Job system debug window if enabled in config
    std::unique_ptr<DebugJobWindow> jobWindow;
    ShowJobMetrics = false;

    // Player info debug window (created later after player is initialized)
    std::unique_ptr<DebugPlayerInfoWindow> playerInfoWindow;
    ShowPlayerInfo = false;

    // Coordinator debug window (created later after OnScreenChecks is initialized)
    std::unique_ptr<DebugCoordinatorWindow> coordinatorWindow;
    ShowCoordinatorInfo = false;

    // Input info debug window
    std::unique_ptr<DebugInputInfoWindow> inputInfoWindow;
    ShowInputInfo = false;

    // Character info debug windows (created on click)
    std::vector<std::unique_ptr<DebugCharacterInfoWindow>> characterInfoWindows;

    if (windowConfig.contains("Debug"))
    {
        auto &debug = windowConfig["Debug"];

        if (debug.contains("ShowFPSMetrics"))
        {
            ShowFPSMetrics = debug["ShowFPSMetrics"];
            printf("Debug ShowFPSMetrics: %s\n", ShowFPSMetrics ? "enabled" : "disabled");

            if (ShowFPSMetrics)
            {
                fpsWindow = std::make_unique<DebugFPSWindow>("FPS Metrics");
                printf("Created FPS metrics debug window\n");
            }
        }

        if (debug.contains("ShowJobMetrics"))
        {
            ShowJobMetrics = debug["ShowJobMetrics"];
            printf("Debug ShowJobMetrics: %s\n", ShowJobMetrics ? "enabled" : "disabled");

            if (ShowJobMetrics)
            {
                jobWindow = std::make_unique<DebugJobWindow>("Job System");
                printf("Created Job system debug window\n");
            }
        }

        if (debug.contains("ShowPlayerInfo"))
        {
            ShowPlayerInfo = debug["ShowPlayerInfo"];
            printf("Debug ShowPlayerInfo: %s\n", ShowPlayerInfo ? "enabled" : "disabled");
        }

        if (debug.contains("ShowCoordinatorInfo"))
        {
            ShowCoordinatorInfo = debug["ShowCoordinatorInfo"];
            printf("Debug ShowCoordinatorInfo: %s\n", ShowCoordinatorInfo ? "enabled" : "disabled");
        }

        if (debug.contains("ShowInputInfo"))
        {
            ShowInputInfo = debug["ShowInputInfo"];
            printf("Debug ShowInputInfo: %s\n", ShowInputInfo ? "enabled" : "disabled");

            if (ShowInputInfo)
            {
                inputInfoWindow = std::make_unique<DebugInputInfoWindow>("Input Info");
                printf("Created Input info debug window\n");
            }
        }

        if (debug.contains("ShowAtlasLabeler"))
        {
            // this allows us to map numbered rectangles to named items in the atlas JSON file.
            atlas_labeler_open = debug["ShowAtlasLabeler"];
            printf("Debug ShowAtlasLabeler: %s\n", atlas_labeler_open ? "enabled" : "disabled");
        }
    }

    debugSceneSwitcherWindow = std::make_unique<DebugSceneSwitcherWindow>("Scene Switcher");

    // Input recording setup
    bool recordInputInfo = false;
    std::ofstream inputLogFile;
    if (windowConfig.contains("Debug") && windowConfig["Debug"].contains("RecordInputInfo"))
    {
        recordInputInfo = windowConfig["Debug"]["RecordInputInfo"];
        if (recordInputInfo)
        {
            inputLogFile.open("input_log.txt", std::ios::out | std::ios::trunc);
            if (inputLogFile.is_open())
            {
                printf("Input recording enabled - logging to input_log.txt\n");
                inputLogFile << "Frame,DeltaTime,KeyW,KeyS,KeyA,KeyD,JoyCount,StickX,StickY,StickMag,MoveVecX,MoveVecY,MoveMag,Source\n";
            }
            else
            {
                printf("Warning: Failed to open input_log.txt for writing\n");
                recordInputInfo = false;
            }
        }
    }
}

void SceneManager::Setup()
{
    if (currentScene && !currentScene->getHasSetup())
    {
        currentScene->setup();
    }
}

bool SceneManager::MainLoop()
{
    // this is called every frame of the mian loop if app exists

    // manages switching b/t scenes

    // if current scene is set up, run its main loop
    if (currentScene)
    {
        if (!currentScene->getHasSetup())
        {
            // run scene setup this loop
            Setup();
            return true;
        }
        // if the current scene has requested cleanup, perform it before running the main loop
        if (currentScene->getHasRequestedCleanup())
        {
            // clean up the scene instead
            Cleanup();
            return true;
        }
        // if no cleanup is requested, run the main loop of the current scene
        currentScene->mainLoop();
        if (debugSceneSwitcherWindow)
        {
            const std::string requestedScene = debugSceneSwitcherWindow->takeRequestedScene();
            if (!requestedScene.empty())
            {
                LoadScene(requestedScene);
            }
        }
        return true;
    }
    else
    {
        // current scene is null, load nextScene if available
        LoadScene(nextScene);
        nextScene.clear();
        return true;
    }
    return false;
}

void SceneManager::Cleanup()
{
    if (currentScene && currentScene->getHasSetup())
    {
        currentScene->cleanup();
    }
}

void SceneManager::SharedCleanup()
{
    if (currentScene && currentScene->getHasSetup())
    {
        Cleanup();
    }
    // Shutdown the job system after cleaning up the current scene as it will be cycled
    JobSystem::shutdown();
}

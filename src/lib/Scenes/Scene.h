#pragma once

#include "AnimatedDataCharacterNavMeshPlayer.h"
#include "AtlasLabelerWindow.h"
#include "DataFile.h"
#include "DebugWindowList.h"
#include "HudUI.h"
#include "LevelV1.h"
#include <cute.h>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

class DebugCharacterInfoWindow;
class DebugPlayerInfoWindow;
class DebugCoordinatorWindow;
class DebugFPSWindow;
class DebugJobWindow;
class DebugInputInfoWindow;
class CFNativeCamera;

// Represents a scene loaded from assets/DataFiles/Scenes/<name>.json
class Scene
{
public:
    Scene() = default;
    explicit Scene(const std::string &sceneName);

    // Load assets/DataFiles/Scenes/<sceneName>.json and populate name/type/level from it
    bool init(const std::string &sceneName);

    const std::string &getName() const;
    const std::string &getType() const;
    const std::string &getLevel() const;
    bool getIsValid() const;

    void setup();
    int setupLevel();
    int setupMenu();

    void mainLoop();
    void mainLoopLevel();
    void mainLoopMenu();

    void cleanup();
    void cleanupLevel();
    void cleanupMenu();

    void RequestCleanup(); // called by self

    bool getHasRequestedCleanup() const;
    bool getHasSetup() const;

    // Setters used by SceneManager to populate this scene's level-scene vars from its
    // own shared instances.
    void setWindowConfig(const DataFile &value) { windowConfig = value; }
    void setViewportWidth(float value) { viewportWidth = value; }
    void setViewportHeight(float value) { viewportHeight = value; }
    void setViewportZoom(float value) { viewportZoom = value; }
    void setDebugHighlightViewport(bool value) { debugHighlightViewport = value; }
    void setDebugHighlightNavmesh(bool value) { debugHighlightNavmesh = value; }
    void setDebugHighlightNavMeshPaths(bool value) { debughighlightNavMeshPaths = value; }
    void setDebugHighlightAgents(bool value) { debugHighlightAgents = value; }
    void setDebugHighlightCharacterHitboxes(bool value) { debugHighlightCharacterHitboxes = value; }
    void setDebugHighlightSpatialGrid(bool value) { debugHighlightSpatialGrid = value; }
    void setDebugHighlightCoordinatorInfo(bool value) { debugHighlightCoordinatorInfo = value; }
    void setDebugHighlightPlayerNavmeshCollisionBox(bool value) { debugHighlightPlayerNavmeshCollisionBox = value; }
    void setClickToInspectCharacter(bool value) { clickToInspectCharacter = value; }
    void setDebugEntityScale(float value) { debugEntityScale = value; }
    void setDebugWindows(DebugWindowList &value) { debugWindows = std::move(value); }
    void setAtlasLabeler(const AtlasLabelerWindow &value) { atlasLabeler = value; }
    void setAtlasLabelerOpen(bool value) { atlas_labeler_open = value; }
    void setShowFPSMetrics(bool value) { ShowFPSMetrics = value; }
    void setShowJobMetrics(bool value) { ShowJobMetrics = value; }
    void setShowPlayerInfo(bool value) { ShowPlayerInfo = value; }
    void setShowCoordinatorInfo(bool value) { ShowCoordinatorInfo = value; }
    void setShowInputInfo(bool value) { ShowInputInfo = value; }
    void setShowNavMesh(bool value) { showNavMesh = value; }
    void setShowNavMeshPoints(bool value) { showNavMeshPoints = value; }
    void setShowAgents(bool value) { showAgents = value; }
    void setCharacterInfoWindows(std::vector<std::unique_ptr<DebugCharacterInfoWindow>> &value) { characterInfoWindows = std::move(value); }
    void setRecordInputInfo(bool value) { recordInputInfo = value; }
    void setInputLogFile(std::ofstream &value) { inputLogFile = std::move(value); }
    // Camera is owned by SceneManager so it survives scene swaps; this just points at it.
    void setCfCamera(CFNativeCamera &value) { cfCamera = &value; }

private:
    std::string level_directory = "assets/Levels/";
    DataFile datafile;
    std::string name;
    std::string type;
    std::string level_name; // Only present for scenes of type "level"
    bool isValid = false;
    bool hasSetup = false;
    bool hasRequestedCleanup = false;
    // vars used by Level scenes, populated by SceneManager via the setters above.
    DataFile windowConfig;
    float viewportWidth = 0.0f;
    float viewportHeight = 0.0f;
    float viewportZoom = 0.0f;
    bool debugHighlightViewport = false;
    bool debugHighlightNavmesh = false;
    bool debughighlightNavMeshPaths = false;
    bool debugHighlightAgents = false;
    bool debugHighlightCharacterHitboxes = false;
    bool debugHighlightSpatialGrid = false;
    bool debugHighlightCoordinatorInfo = false;
    bool debugHighlightPlayerNavmeshCollisionBox = false;
    bool clickToInspectCharacter = false;
    float debugEntityScale = 0.0f;
    DebugWindowList debugWindows;
    AtlasLabelerWindow atlasLabeler;
    bool atlas_labeler_open = false;
    bool ShowFPSMetrics = false;
    bool ShowJobMetrics = false;
    bool ShowPlayerInfo = false;
    bool ShowCoordinatorInfo = false;
    bool ShowInputInfo = false;
    bool showAgents = false;
    std::vector<std::unique_ptr<DebugCharacterInfoWindow>> characterInfoWindows;
    bool recordInputInfo = false;
    std::ofstream inputLogFile;
    bool showNavMeshPoints;
    bool showNavMesh;
    bool hud_inventory_open = false;
    // objects used by level scenes
    std::unique_ptr<LevelV1> level;
    std::unique_ptr<DebugPlayerInfoWindow> playerInfoWindow;
    std::unique_ptr<DebugCoordinatorWindow> coordinatorWindow;
    std::unique_ptr<DebugFPSWindow> fpsWindow;
    std::unique_ptr<DebugJobWindow> jobWindow;
    std::unique_ptr<DebugInputInfoWindow> inputInfoWindow;
    CFNativeCamera *cfCamera = nullptr; // owned by SceneManager, shared via setCfCamera
    AnimatedDataCharacterNavMeshPlayer playerCharacter;
    HudUI hud_ui;
    float startTileX = 0.0f;
    float startTileY = 0.0f;
    float startWorldX = 0.0f;
    float startWorldY = 0.0f;
    v2 playerPosition;
};

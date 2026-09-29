#pragma once

#include "CFNativeCamera.h"
#include "Scene.h"
#include <memory>
#include <string>

class DebugSceneSwitcherWindow;

class SceneManager
{
public:
    SceneManager();
    ~SceneManager();

    // Load a new scene from the given scene name. Returns true if successful.
    bool LoadScene(const std::string &scene_name);
    // get info
    Scene *GetcurrentScene();
    const Scene *GetcurrentScene() const;

    void SharedSetup(int windowWidth, int windowHeight);
    void Setup();
    bool MainLoop(); // return true if app should keep running
    void Cleanup();
    void SharedCleanup();

private:
    // used for scene switching
    std::unique_ptr<Scene> currentScene;
    std::string nextScene; // name of next scene to load, if current scene is suddenly null (used for level transisions)
    // used in level type scenes
    DataFile windowConfig;
    CFNativeCamera cfCamera; // owned here so it survives scene swaps
    float viewportWidth;
    float viewportHeight;
    float viewportZoom;
    bool debugHighlightViewport;
    bool debugHighlightNavmesh;
    bool debughighlightNavMeshPaths;
    bool debugHighlightAgents;
    bool debugHighlightCharacterHitboxes;
    bool debugHighlightSpatialGrid;
    bool debugHighlightCoordinatorInfo;
    bool debugHighlightPlayerNavmeshCollisionBox;
    bool clickToInspectCharacter;
    bool clickToCreateEntity;
    bool debugHighlightTileUnderCursor;
    float debugEntityScale;
    std::unique_ptr<DebugSceneSwitcherWindow> debugSceneSwitcherWindow;
    DebugWindowList debugWindows;
    AtlasLabelerWindow atlasLabeler;
    bool atlas_labeler_open;
    bool ShowFPSMetrics;
    bool ShowJobMetrics;
    bool ShowPlayerInfo;
    bool ShowCoordinatorInfo;
    bool ShowInputInfo;
    std::vector<std::unique_ptr<DebugCharacterInfoWindow>> characterInfoWindows;
    bool recordInputInfo;
    std::ofstream inputLogFile;
};

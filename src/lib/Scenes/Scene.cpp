#include "Scene.h"
#include <cstdio>
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

Scene::Scene(const std::string &sceneName)
{
    init(sceneName);
}

bool Scene::init(const std::string &sceneName)
{
    std::string scenePath = "assets/DataFiles/Scenes/" + sceneName + ".json";

    if (!datafile.load(scenePath))
    {
        printf("Scene: ERROR: Failed to load scene '%s' from '%s'\n", sceneName.c_str(), scenePath.c_str());
        isValid = false;
        return false;
    }

    if (!datafile.contains("name") || !datafile.contains("type"))
    {
        printf("Scene: ERROR: '%s' missing required fields (name or type)\n", scenePath.c_str());
        isValid = false;
        return false;
    }

    name = datafile["name"].get<std::string>();
    type = datafile["type"].get<std::string>();

    // Only "level" scenes reference a level to load
    level_name = (datafile.contains("level") && datafile["level"].is_string()) ? datafile["level"].get<std::string>() : "";

    isValid = true;
    return true;
}

const std::string &Scene::getName() const
{
    return name;
}

const std::string &Scene::getType() const
{
    return type;
}

const std::string &Scene::getLevel() const
{
    return level_name;
}

bool Scene::getIsValid() const
{
    return isValid;
}
void Scene::setup()
{
    hasSetup = true;
    if (type == "level")
    {
        setupLevel();
    }
    else if (type == "menu")
    {
        setupMenu();
    }
}

int Scene::setupLevel()
{
    // Implement level setup logic here

    // Scene setup
    //=====================================================================================================
    //=====================================================================================================
    //=====================================================================================================
    //=====================================================================================================
    // Create LevelV1 instance - handles all TMX and NavMesh initialization
    level = std::make_unique<LevelV1>(level_directory + level_name);

    if (!level->isInitialized())
    {
        printf("Error: Failed to initialize level\n");
        destroy_app();
        return -1;
    }

    // Configure layer highlighting from config (parse once, use map for lookups)
    level->getLevelMap().setLayerHighlightConfig(windowConfig);

    // Get tile dimensions from level for proper spacing
    int tile_width = level->getTileWidth();
    int tile_height = level->getTileHeight();

    // Create playerCharacter player character
    // AnimatedDataCharacterNavMeshPlayer playerCharacter;
    // HudUI hud_ui;

    // Starting position in tile coordinates (will be converted to world coordinates)
    startTileX = 5.0f;
    startTileY = 10.0f;
    if (level->start_tile_x >= 0)
    {
        startTileX = static_cast<float>(level->start_tile_x);
    }
    if (level->start_tile_y >= 0)
    {
        startTileY = static_cast<float>(level->start_tile_y);
    }

    // Convert tile coordinates to world pixel coordinates
    startWorldX = startTileX * tile_width;
    startWorldY = startTileY * tile_height;

    playerPosition = cf_v2(startWorldX, startWorldY);

    printf("Player starting at tile (%.1f, %.1f) = world (%.1f, %.1f)\n",
           startTileX, startTileY, startWorldX, startWorldY);

    if (!playerCharacter.init("assets/DataFiles/Entities/player"))
    {
        destroy_app();
        return -1;
    }

    // add debug items to the player
    Item itemA("assets/DataFiles/Items/item-a.json");
    Item itemB("assets/DataFiles/Items/item-b.json");
    playerCharacter.getInventory().addItem(itemA);
    playerCharacter.getInventory().addItem(itemB);
    bool hud_inventory_open = false;

    // HUD init (loads atlas/background)
    hud_ui.initialize();

    // Set player's initial position
    playerCharacter.setPosition(playerPosition);

    // Connect player to level for hitbox collision detection
    playerCharacter.setLevel(level.get());

    // Set player reference in level for action hitbox checking
    level->setPlayer(&playerCharacter);

    // Connect player to navmesh for walkable area detection
    playerCharacter.setNavMesh(&level->getNavMesh());

    // Set sprite dimensions for navmesh collision box calculation (tile size is the sprite size)
    playerCharacter.setSpriteDimensions(static_cast<float>(tile_width), static_cast<float>(tile_height));

    // Set initial hitbox visibility from config for player
    playerCharacter.sethitboxDebugActive(debugHighlightCharacterHitboxes);
    playerCharacter.setEntityScale(debugEntityScale);

    // Set hitbox visibility for all agents from config
    for (size_t i = 0; i < level->getAgentCount(); ++i)
    {
        AnimatedDataCharacterNavMeshAgent *agent = level->getAgent(i);
        if (agent)
        {
            agent->sethitboxDebugActive(debugHighlightCharacterHitboxes);
            agent->setEntityScale(debugEntityScale);
        }
    }

    // Create player info debug window if enabled (now that player and level are ready)
    if (ShowPlayerInfo)
    {
        playerInfoWindow = std::make_unique<DebugPlayerInfoWindow>("Player Info", playerCharacter, *level);
        printf("Created Player info debug window\n");
    }

    // Create FPS metrics debug window if enabled
    if (ShowFPSMetrics)
    {
        fpsWindow = std::make_unique<DebugFPSWindow>("FPS Metrics");
        printf("Created FPS metrics debug window\n");
    }

    // Create job system debug window if enabled
    if (ShowJobMetrics)
    {
        jobWindow = std::make_unique<DebugJobWindow>("Job Metrics");
        printf("Created Job metrics debug window\n");
    }

    // Create input info debug window if enabled
    if (ShowInputInfo)
    {
        inputInfoWindow = std::make_unique<DebugInputInfoWindow>("Input Info");
        printf("Created Input info debug window\n");
    }

    // Camera is owned by SceneManager (set via setCfCamera) so it survives scene swaps.
    // Configure it here for this scene's viewport/zoom and player-following behavior.
    cfCamera->setViewportSize(viewportWidth, viewportHeight);
    cfCamera->setZoom(viewportZoom);
    cfCamera->setZoomRange(0.25f, 4.0f); // Allow 1/4x to 4x zoom

    // Make camera follow the player
    cfCamera->setTarget(&playerPosition);
    cfCamera->setFollowSpeed(3.0f);
    // Scale deadzone with zoom to maintain consistent feel (base 50px at 1.0 zoom)
    float deadzoneSize = 50.0f / viewportZoom;
    cfCamera->setFollowDeadzone(cf_v2(deadzoneSize, deadzoneSize));

    // get height and width of window for where to draw debug info
    int window_width = cf_app_get_width();
    int window_height = cf_app_get_height();
    float bottom_left_x = -1.0f * (window_width / 2.0f);
    float bottom_left_y = -1.0f * (window_height / 2.0f);
    // printf("Window size: %dx%d, bottom-left at (%.1f, %.1f)\n", window_width, window_height, bottom_left_x, bottom_left_y);

    // NavMesh debug rendering toggle (initialized from config)
    bool showNavMesh = debugHighlightNavmesh;
    bool showNavMeshPoints = debughighlightNavMeshPaths;
    bool showAgents = debugHighlightAgents;

    // NavMesh path for pathfinding (stored as shared_ptr)
    std::shared_ptr<NavMeshPath> navmeshPath = nullptr;

    // Initialize and start on-screen checks worker
    OnScreenChecks::initialize(&playerPosition, cfCamera, level.get(), &playerCharacter);
    OnScreenChecks::start();

    // Create coordinator debug window if enabled (now that OnScreenChecks is initialized)
    if (ShowCoordinatorInfo)
    {
        coordinatorWindow = std::make_unique<DebugCoordinatorWindow>("Coordinator Info",
                                                                     OnScreenChecks::getCoordinator(),
                                                                     &playerCharacter,
                                                                     *level);
        printf("Created Coordinator debug window\n");
    }
    return 0;
}

int Scene::setupMenu()
{
    // Implement menu setup logic here
    return 0;
}

void Scene::mainLoop()
{
    if (type == "level")
    {
        mainLoopLevel();
    }
    else if (type == "menu")
    {
        mainLoopMenu();
    }
}

void Scene::mainLoopLevel()
{
    // Implement level main loop logic here
    //============================================================================================
    //============================================================================================
    //============================================================================================
    //============================================================================================
    // Begin profiling the frame
    if (fpsWindow)
    {
        fpsWindow->beginFrame();
    }

    // Update app to handle window events and input (proper CF pattern)
    cf_app_update(NULL);

    // Handle ESC to quit
    // if (cf_key_just_pressed(CF_KEY_ESCAPE))
    //{
    //    break;
    //}

    // Handle mouse click for character inspection
    if (clickToInspectCharacter && cf_mouse_just_pressed(CF_MOUSE_BUTTON_LEFT))
    {
        // Get mouse position in window coordinates
        float mouseWindowX = cf_mouse_x();
        float mouseWindowY = cf_mouse_y();

        // Get current window dimensions
        int windowWidth = cf_app_get_width();
        int windowHeight = cf_app_get_height();

        // Get viewport dimensions from camera
        v2 viewportSize = cfCamera->getViewportSize();

        // Calculate viewport offset (assuming viewport is centered in window)
        float viewportOffsetX = (windowWidth - viewportSize.x) * 0.5f;
        float viewportOffsetY = (windowHeight - viewportSize.y) * 0.5f;

        // Convert window coordinates to viewport coordinates
        float mouseViewportX = mouseWindowX - viewportOffsetX;
        float mouseViewportY = mouseWindowY - viewportOffsetY;

        // Check if mouse is within viewport bounds
        if (mouseViewportX < 0 || mouseViewportX >= viewportSize.x ||
            mouseViewportY < 0 || mouseViewportY >= viewportSize.y)
        {
            printf("Mouse Click - Outside viewport bounds\n");
        }
        else
        {
            // Convert viewport to world coordinates
            // CF uses a coordinate system where (0,0) is at the center of the viewport
            // Mouse coordinates are top-left origin, so we need to convert:
            // 1. Convert viewport coords from top-left origin to center origin
            v2 centered = cf_v2(mouseViewportX - viewportSize.x * 0.5f,
                                -(mouseViewportY - viewportSize.y * 0.5f)); // Negate Y to flip from Y-down to Y-up
            // 2. Apply inverse zoom
            float zoom = cfCamera->getZoom();
            centered = cf_v2(centered.x / zoom, centered.y / zoom);
            // 3. Add camera position to get world coordinates
            v2 cameraPos = cfCamera->getPosition();
            v2 mouseWorldPos = cf_v2(centered.x + cameraPos.x, centered.y + cameraPos.y);

            // Convert world coordinates to tile coordinates using the level's TMX conversion
            int tmxTileX, tmxTileY;
            if (level->getLevelMap().worldToMapCoords(mouseWorldPos.x, mouseWorldPos.y, 0.0f, 0.0f, tmxTileX, tmxTileY))
            {
                // Convert TMX coordinates (Y-down, 0 = top) to rendering coordinates (Y-up, 0 = bottom)
                // This matches what highlightTile() expects
                int mapHeight = level->getLevelMap().getMapHeight();
                int renderTileX = tmxTileX;
                int renderTileY = mapHeight - 1 - tmxTileY;

                printf("Mouse Click - Screen: (%.1f, %.1f) | World: (%.1f, %.1f) | Tile: (%d, %d)\n",
                       mouseWindowX, mouseWindowY, mouseWorldPos.x, mouseWorldPos.y, renderTileX, renderTileY);

                // Get all entities at this tile
                auto entities = level->get_entities_at(renderTileX, renderTileY);
                if (entities.empty())
                {
                    printf("  No entities at tile (%d, %d)\n", renderTileX, renderTileY);
                }
                else
                {
                    printf("  Entities at tile (%d, %d):\n", renderTileX, renderTileY);
                    for (auto *entity : entities)
                    {
                        printf("    - %s\n", entity->getDataFilePath().c_str());

                        // Check if we already have a window tracking this entity
                        bool alreadyTracking = false;
                        for (const auto &window : characterInfoWindows)
                        {
                            if (window->isTracking(entity))
                            {
                                alreadyTracking = true;
                                break;
                            }
                        }

                        // Create a new debug window for this entity if not already tracking
                        if (!alreadyTracking)
                        {
                            std::string windowTitle = "Character Info: " + entity->getDataFilePath();
                            auto newWindow = std::make_unique<DebugCharacterInfoWindow>(windowTitle, entity, *level);
                            characterInfoWindows.push_back(std::move(newWindow));
                            printf("      Created debug window for entity\n");
                        }
                        else
                        {
                            printf("      Already tracking this entity\n");
                        }
                    }
                }
            }
            else
            {
                printf("Mouse Click - Screen: (%.1f, %.1f) | World: (%.1f, %.1f) | Tile: OUT OF BOUNDS\n",
                       mouseWindowX, mouseWindowY, mouseWorldPos.x, mouseWorldPos.y);
            }
        }
    }

    // Player movement (WASD + Controller) - only one direction at a time, most recent key takes priority
    float dt = CF_DELTA_TIME;
    float playerSpeed = 200.0f; // pixels per second

    // Track which direction key was most recently pressed
    // 0 = none, 1 = up, 2 = down, 3 = left, 4 = right
    static int lastPressedDirection = 0;

    // Check for newly pressed keys (just_pressed) to update priority
    if (cf_key_just_pressed(CF_KEY_W) || cf_key_just_pressed(CF_KEY_UP))
    {
        lastPressedDirection = 1; // up
    }
    if (cf_key_just_pressed(CF_KEY_S) || cf_key_just_pressed(CF_KEY_DOWN))
    {
        lastPressedDirection = 2; // down
    }
    if (cf_key_just_pressed(CF_KEY_A) || cf_key_just_pressed(CF_KEY_LEFT))
    {
        lastPressedDirection = 3; // left
    }
    if (cf_key_just_pressed(CF_KEY_D) || cf_key_just_pressed(CF_KEY_RIGHT))
    {
        lastPressedDirection = 4; // right
    }

    // If the last pressed key is no longer held, find another held key
    bool lastKeyStillHeld = false;
    switch (lastPressedDirection)
    {
    case 1:
        lastKeyStillHeld = cf_key_down(CF_KEY_W) || cf_key_down(CF_KEY_UP);
        break;
    case 2:
        lastKeyStillHeld = cf_key_down(CF_KEY_S) || cf_key_down(CF_KEY_DOWN);
        break;
    case 3:
        lastKeyStillHeld = cf_key_down(CF_KEY_A) || cf_key_down(CF_KEY_LEFT);
        break;
    case 4:
        lastKeyStillHeld = cf_key_down(CF_KEY_D) || cf_key_down(CF_KEY_RIGHT);
        break;
    default:
        lastKeyStillHeld = false;
        break;
    }

    if (!lastKeyStillHeld)
    {
        // Find another key that's still held (priority: W, S, A, D)
        if (cf_key_down(CF_KEY_W) || cf_key_down(CF_KEY_UP))
            lastPressedDirection = 1;
        else if (cf_key_down(CF_KEY_S) || cf_key_down(CF_KEY_DOWN))
            lastPressedDirection = 2;
        else if (cf_key_down(CF_KEY_A) || cf_key_down(CF_KEY_LEFT))
            lastPressedDirection = 3;
        else if (cf_key_down(CF_KEY_D) || cf_key_down(CF_KEY_RIGHT))
            lastPressedDirection = 4;
        else
            lastPressedDirection = 0; // No keys held
    }

    // Calculate move vector based on the single active direction
    v2 moveVector = cf_v2(0.0f, 0.0f);
    switch (lastPressedDirection)
    {
    case 1:
        moveVector.y = playerSpeed;
        break; // up
    case 2:
        moveVector.y = -playerSpeed;
        break; // down
    case 3:
        moveVector.x = -playerSpeed;
        break; // left
    case 4:
        moveVector.x = playerSpeed;
        break; // right
    }

    // Controller input - Left stick movement (overrides keyboard if stick is moved)
    bool usingController = false;
    float leftStickX = 0.0f;
    float leftStickY = 0.0f;
    float stickMagnitude = 0.0f;

    if (cf_joypad_count() > 0)
    {
        const float deadzone = 0.2f; // Ignore small stick movements

        // Get raw stick values and normalize them
        // CF returns raw values in range -32768 to 32767, we need -1.0 to 1.0
        float rawStickX = cf_joypad_axis(0, CF_JOYPAD_AXIS_LEFTX);
        float rawStickY = cf_joypad_axis(0, CF_JOYPAD_AXIS_LEFTY);

        // Normalize to -1.0 to 1.0 range
        leftStickX = rawStickX / 32767.0f;
        leftStickY = rawStickY / 32767.0f;

        // Check if stick is outside deadzone
        stickMagnitude = sqrtf(leftStickX * leftStickX + leftStickY * leftStickY);
        if (stickMagnitude > deadzone)
        {
            // Use stick input directly for movement
            // Y-axis is inverted on controllers (positive = down), so negate it
            moveVector.x = leftStickX * playerSpeed;
            moveVector.y = -leftStickY * playerSpeed; // Negate Y to fix inversion
            usingController = true;
        }
    }

    // Record input info if enabled
    if (recordInputInfo && inputLogFile.is_open())
    {
        static int frameNumber = 0;
        float moveMagnitude = sqrtf(moveVector.x * moveVector.x + moveVector.y * moveVector.y);

        bool keyW = cf_key_down(CF_KEY_W) || cf_key_down(CF_KEY_UP);
        bool keyS = cf_key_down(CF_KEY_S) || cf_key_down(CF_KEY_DOWN);
        bool keyA = cf_key_down(CF_KEY_A) || cf_key_down(CF_KEY_LEFT);
        bool keyD = cf_key_down(CF_KEY_D) || cf_key_down(CF_KEY_RIGHT);

        inputLogFile << frameNumber++ << ","
                     << dt << ","
                     << keyW << "," << keyS << "," << keyA << "," << keyD << ","
                     << cf_joypad_count() << ","
                     << leftStickX << "," << leftStickY << "," << stickMagnitude << ","
                     << moveVector.x << "," << moveVector.y << "," << moveMagnitude << ","
                     << (usingController ? "Controller" : "Keyboard") << "\n";
    }

    // Handle playerCharacter animation input (1/2 for idle/walk)
    // playerCharacter.handleInput();

    // Check if player has an active action and if it's in warmup
    Action *activeAction = playerCharacter.getActiveAction();
    bool playerInWarmup = false;
    if (activeAction)
    {
        // Action is in warmup if it's active but not in cooldown
        playerInWarmup = !activeAction->getInCooldown();
    }

    // Only allow action inputs if player is not in warmup
    if (!playerInWarmup)
    {
        // Handle spacebar or controller A button to trigger action A
        if (cf_key_just_pressed(CF_KEY_SPACE) ||
            (cf_joypad_count() > 0 && cf_joypad_button_just_pressed(0, CF_JOYPAD_BUTTON_A)))
        {
            Action *actionA = playerCharacter.getActionPointerA();
            if (actionA)
            {
                actionA->doAction();
                printf("Player triggered action A\n");
            }
        }

        // Handle B key or controller B button to trigger action B
        if (cf_key_just_pressed(CF_KEY_B) ||
            (cf_joypad_count() > 0 && cf_joypad_button_just_pressed(0, CF_JOYPAD_BUTTON_B)))
        {
            Action *actionB = playerCharacter.getActionPointerB();
            if (actionB)
            {
                actionB->doAction();
                printf("Player triggered action B\n");
            }
        }
    }
    if (!playerCharacter.getIsDoingAction())
    {
        // Track trigger states for edge detection
        static bool leftTriggerWasPressed = false;
        static bool rightTriggerWasPressed = false;

        bool leftTriggerPressed = (cf_joypad_count() > 0 && cf_joypad_axis(0, CF_JOYPAD_AXIS_TRIGGERLEFT) > 0.5f);
        bool rightTriggerPressed = (cf_joypad_count() > 0 && cf_joypad_axis(0, CF_JOYPAD_AXIS_TRIGGERRIGHT) > 0.5f);

        // Handle action pointer A navigation (I/O keys or left/right bumpers)
        if (cf_key_just_pressed(CF_KEY_I) ||
            (cf_joypad_count() > 0 && cf_joypad_button_just_pressed(0, CF_JOYPAD_BUTTON_LEFTSHOULDER)))
        {
            playerCharacter.MoveActionPointerADown(); // Move towards index 0
        }
        if (cf_key_just_pressed(CF_KEY_O) ||
            (cf_joypad_count() > 0 && cf_joypad_button_just_pressed(0, CF_JOYPAD_BUTTON_RIGHTSHOULDER)))
        {
            playerCharacter.MoveActionPointerAUp(); // Move towards end of list
        }

        // Handle action pointer B navigation (K/L keys or left/right triggers)
        if (cf_key_just_pressed(CF_KEY_K) || (leftTriggerPressed && !leftTriggerWasPressed))
        {
            playerCharacter.MoveActionPointerBDown(); // Move towards index 0
        }
        if (cf_key_just_pressed(CF_KEY_L) || (rightTriggerPressed && !rightTriggerWasPressed))
        {
            playerCharacter.MoveActionPointerBUp(); // Move towards end of list
        }

        // Update trigger states for next frame
        leftTriggerWasPressed = leftTriggerPressed;
        rightTriggerWasPressed = rightTriggerPressed;
    }

    // Camera feature demo keys
    if (cf_key_just_pressed(CF_KEY_T))
    {
        cfCamera->moveTo(cf_v2(playerPosition.x + 200.0f, playerPosition.y + 200.0f), 2.0f);
    }
    if (cf_key_just_pressed(CF_KEY_Y))
    {
        cfCamera->zoomTo(2.0f, 1.5f);
    }
    if (cf_key_just_pressed(CF_KEY_U))
    {
        cfCamera->shake(20.0f, 1.5f);
    }

    // NavMesh visualization toggle
    if (cf_key_just_pressed(CF_KEY_N))
    {
        showNavMesh = !showNavMesh;
        printf("NavMesh visualization: %s\n", showNavMesh ? "ON" : "OFF");
    }

    // NavMesh points visualization toggle
    if (cf_key_just_pressed(CF_KEY_M))
    {
        showNavMeshPoints = !showNavMeshPoints;
        printf("NavMesh points visualization: %s\n", showNavMeshPoints ? "ON" : "OFF");
    }

    // Place/update NavMesh point at player position
    if (cf_key_just_pressed(CF_KEY_P))
    {
        // Remove existing point if it exists
        if (level->getNavMesh().getPoint("player_marker") != nullptr)
        {
            level->getNavMesh().removePoint("player_marker");
        }
        // Add new point at player position
        level->getNavMesh().addPoint("player_marker", playerPosition);
        printf("NavMesh point placed at player position (%.1f, %.1f)\n", playerPosition.x, playerPosition.y);
    }

    // Camera zoom controls (Q/E) and reset (R)
    if (cf_key_just_pressed(CF_KEY_Q))
    {
        cfCamera->zoomOut(1.2f);
    }
    if (cf_key_just_pressed(CF_KEY_E))
    {
        cfCamera->zoomIn(1.2f);
    }
    if (cf_key_just_pressed(CF_KEY_R))
    {
        cfCamera->reset();
    }

    // Trigger red flash effect on the player with 'F'
    if (cf_key_just_pressed(CF_KEY_F))
    {
        playerCharacter.triggerEffect("red", 3, 2.0f, 0.85f);
    }
    // Trigger green flash effect on the player with 'G' (replace any current effect)
    if (cf_key_just_pressed(CF_KEY_G))
    {
        playerCharacter.triggerEffect("green", 3, 2.0f, 0.85f);
    }
    // Trigger dissolve effect on the player with 'X'
    if (cf_key_just_pressed(CF_KEY_X))
    {
        // flashes unused, duration ~1.0s, edgeWidth ~0.06
        playerCharacter.triggerEffect("dissolve", 1, 1.0f, 0.06f);
    }
    // Toggle trail ghost effect on the player with 'H'
    if (cf_key_just_pressed(CF_KEY_H))
    {
        // Parameters: ghosts, duration, base alpha
        playerCharacter.triggerEffect("trail", 8, 1.5f, 0.8f);
    }
    // Toggle sample inventory window
    if (cf_key_just_pressed(CF_KEY_I))
    {
        hud_inventory_open = !hud_inventory_open;
    }

    if (fpsWindow)
    {
        fpsWindow->markSection("Player Input");
    }
    level->updateAgents(dt);

    if (fpsWindow)
    {
        fpsWindow->markSection("Agent Update");
    }

    // Update playerCharacter animation with move vector
    playerCharacter.update(dt, moveVector);

    // Get updated player position from playerCharacter (for camera following)
    playerPosition = playerCharacter.getPosition();
    if (fpsWindow)
    {
        fpsWindow->markSection("Player Update");
    }
    // Update camera (handles following and smooth movement)
    cfCamera->update(dt);

    if (fpsWindow)
    {
        fpsWindow->markSection("Camera Update");
    }
    // Render debug windows
    debugWindows.renderAll();
    if (debugSceneSwitcherWindow)
    {
        debugSceneSwitcherWindow->render();
    }

    if (atlas_labeler_open)
    {
        atlasLabeler.render();
    }
    // Render HUD UI overlays
    std::vector<HudUI::Icon> hud_icons(5);
    hud_ui.renderLeftColumn(hud_icons, 0.0f, 8.0f);
    hud_ui.renderBottomRow(hud_icons, 0.0f, 8.0f);
    hud_ui.renderInventoryWindow(&playerCharacter.getInventory(), &hud_inventory_open, 5, 64.0f, 6.0f);

    if (fpsWindow)
    {
        fpsWindow->markSection("Debug Windows");
    }
    // Clear background
    CF_Color bg = make_color(0.1f, 0.1f, 0.15f, 1.0f);
    cf_draw_push_color(bg);
    cf_draw_quad_fill(make_aabb(cf_v2(0.0f, 0.0f), (float)cf_app_get_width(), (float)cf_app_get_height()), 0.0f);
    cf_draw_pop_color();

    // Apply CF-native camera transformation for world-space rendering
    cfCamera->apply();

    // WORLD space drawing here (affected by camera)
    v2 text_position1 = cf_v2(0.0f, 0.0f); // World origin
    draw_text("Skeleton Adventure - TMX Level Map", text_position1);

    // Render everything: tiles, action hitboxes, agents, and player
    level->render(*cfCamera, windowConfig, &playerCharacter, 0.0f, 0.0f);

    if (fpsWindow)
    {
        fpsWindow->markSection("Level Render");
    }
    // Render NavMesh debug visualization (if enabled)
    if (showNavMesh && level->getNavMesh().getPolygonCount() > 0)
    {
        level->getNavMesh().debugRender(*cfCamera);
    }

    // Render spatial grid debug visualization (if enabled)
    if (debugHighlightSpatialGrid && level->getSpatialGrid().getOccupiedCellCount() > 0)
    {
        level->getSpatialGrid().debugRender(*cfCamera);
    }

    // Render coordinator grid debug visualization (if enabled)
    if (debugHighlightCoordinatorInfo)
    {
        // Update the coordinator's near-player grid based on player's tile position
        // Use rounding to get the tile the player is centered in, not truncation
        float tileWidth = static_cast<float>(level->getTileWidth());
        float tileHeight = static_cast<float>(level->getTileHeight());
        int playerTileX = static_cast<int>(std::round(playerPosition.x / tileWidth));
        int playerTileY = static_cast<int>(std::round(playerPosition.y / tileHeight));
        OnScreenChecks::getCoordinator()->updateNearPlayerGrid(playerTileX, playerTileY);

        // Render the grid
        OnScreenChecks::getCoordinator()->render();
    }

    // Render NavMesh points debug visualization (if enabled)
    if (showNavMeshPoints && level->getNavMesh().getPointCount() > 0)
    {
        level->getNavMesh().debugRenderPoints(*cfCamera);
    }

    // Render all NavMesh paths (if visualization is enabled)
    if (showNavMeshPoints && level->getNavMesh().getPathCount() > 0)
    {
        const auto &allPaths = level->getNavMesh().getPaths();
        for (const auto &path : allPaths)
        {
            if (path && path->isValid())
            {
                path->debugRender(*cfCamera);
            }
        }
    }

    // Render agent position markers (if enabled)
    if (showAgents && level->getAgentCount() > 0)
    {
        cf_draw_push_color(cf_make_color_rgb(0, 255, 0)); // Green color

        for (size_t i = 0; i < level->getAgentCount(); ++i)
        {
            const AnimatedDataCharacterNavMeshAgent *agent = level->getAgent(i);
            if (agent)
            {
                v2 agentPos = agent->getPosition();

                // Draw agent as a green dot
                const float size = 8.0f; // Size of the agent marker
                CF_Aabb agent_rect = make_aabb(
                    cf_v2(agentPos.x - size / 2, agentPos.y - size / 2),
                    cf_v2(agentPos.x + size / 2, agentPos.y + size / 2));

                cf_draw_quad_fill(agent_rect, 0.0f);

                // Draw a border around the dot for better visibility
                cf_draw_push_color(cf_make_color_rgb(0, 0, 0)); // Black border
                cf_draw_quad(agent_rect, 0.0f, 1.5f);
                cf_draw_pop_color();
            }
        }

        cf_draw_pop_color();
    }

    // Render player's navmesh collision box (if enabled)
    if (debugHighlightPlayerNavmeshCollisionBox)
    {
        playerCharacter.debugRenderNavMeshCollisionBox();
    }

    if (fpsWindow)
    {
        fpsWindow->markSection("Agent/Player Render");
    }
    // Restore camera transformation
    cfCamera->restore();

    // UI space drawing (not affected by camera)
    // Get current window dimensions for proper UI positioning
    int current_width = cf_app_get_width();
    int current_height = cf_app_get_height();
    float top_y = -(current_height / 2.0f) + 20.0f; // 20px from top

    cfCamera->drawDebugInfo(10.0f, top_y);

    // Show player position in UI
    char playerInfo[256];
    snprintf(playerInfo, sizeof(playerInfo), "Player: (%.0f, %.0f)", playerPosition.x, playerPosition.y);
    draw_text(playerInfo, cf_v2(10.0f, top_y + 20.0f));

    // Draw viewport rectangle visualization (if enabled in config)
    if (debugHighlightViewport)
    {
        // Get viewport size from camera
        v2 viewport_size = cfCamera->getViewportSize();

        // Calculate viewport rectangle in screen space (centered)
        float half_vp_width = viewport_size.x / 2.0f;
        float half_vp_height = viewport_size.y / 2.0f;

        CF_Aabb viewport_rect = make_aabb(
            cf_v2(-half_vp_width, -half_vp_height),
            cf_v2(half_vp_width, half_vp_height));

        // Draw viewport boundary as a colored rectangle outline
        cf_draw_push_color(make_color(1.0f, 0.0f, 0.0f, 1.0f)); // Red
        cf_draw_quad(viewport_rect, 0.0f, 3.0f);                // 3px thick outline
        cf_draw_pop_color();

        // Draw viewport info text
        char viewportInfo[256];
        snprintf(viewportInfo, sizeof(viewportInfo), "Viewport: %.0fx%.0f", viewport_size.x, viewport_size.y);
        draw_text(viewportInfo, cf_v2(10.0f, top_y + 40.0f));
    }

    if (fpsWindow)
    {
        fpsWindow->markSection("UI Render");
    }
    // End profiling the frame
    if (fpsWindow)
    {
        fpsWindow->endFrame();
    }
    // Render FPS metrics window if enabled
    if (fpsWindow)
    {
        fpsWindow->render();
    }

    // Render Job system window if enabled
    if (jobWindow)
    {
        jobWindow->render();
    }

    // Render Player info window if enabled
    if (playerInfoWindow)
    {
        playerInfoWindow->render();
    }

    // Render Coordinator info window if enabled
    if (coordinatorWindow)
    {
        coordinatorWindow->render();
    }

    // Render Input info window if enabled
    if (inputInfoWindow)
    {
        inputInfoWindow->render();
    }

    // Render all character info windows
    for (auto &characterWindow : characterInfoWindows)
    {
        characterWindow->render();
    }

    // Remove closed character info windows
    characterInfoWindows.erase(
        std::remove_if(characterInfoWindows.begin(), characterInfoWindows.end(),
                       [](const std::unique_ptr<DebugCharacterInfoWindow> &window)
                       {
                           return !window->isShown();
                       }),
        characterInfoWindows.end());

    // cull dying agents to dead, so they are removed next update loop
    level->cullDyingAgents();

    app_draw_onto_screen();
}

void Scene::mainLoopMenu()
{
    // Implement menu main loop logic here
}

void Scene::cleanup()
{
    if (type == "level")
    {
        cleanupLevel();
    }
    else if (type == "menu")
    {
        cleanupMenu();
    }
}

void Scene::cleanupLevel()
{
    // Implement level cleanup logic here
    // Shutdown on-screen checks worker
    OnScreenChecks::requestShutdown();

    // Shutdown job system
    JobSystem::shutdown();

    // Cleanup on-screen checks
    OnScreenChecks::shutdown();

    // Close input log file if it was opened
    if (inputLogFile.is_open())
    {
        inputLogFile.close();
        printf("Input log file closed\n");
    }
    // start the job system after cleaning up the level
    JobSystem::initialize();
}

void Scene::cleanupMenu()
{
    // Implement menu cleanup logic here
}
void Scene::RequestCleanup()
{
    hasRequestedCleanup = true;
}

bool Scene::getHasRequestedCleanup() const
{
    return hasRequestedCleanup;
}

bool Scene::getHasSetup() const
{
    return hasSetup;
}
#include <cute.h>
#include <stdio.h>
#include <cstdlib>
#include <memory>
#include <dcimgui.h>
#include "DebugWindow.h"
#include "DataFileDebugWindow.h"
#include "DebugWindowList.h"
#include "DebugFPSWindow.h"
#include "DebugJobWindow.h"
#include "DebugPlayerInfoWindow.h"
#include "OnScreenChecks.h"
#include "Utils.h"
#include "DataFile.h"
#include "RealConfigFile.h"
#include "LevelV1.h"
#include "JobSystem.h"

#include "CFNativeCamera.h"
#include "NavMesh.h"
#include "NavMeshPath.h"
#include "AnimatedDataCharacterNavMeshPlayer.h"
#include "HighlightTile.h"
using namespace Cute;

int main(int argc, char *argv[])
{

	// Load window configuration BEFORE creating the window (using RealConfigFile)
	RealConfigFile preConfig("assets/window-config.json");
	int windowWidth = 640;	// Default fallback
	int windowHeight = 480; // Default fallback

	if (preConfig.contains("window"))
	{
		auto &window = preConfig["window"];
		if (window.contains("width") && window.contains("height"))
		{
			windowWidth = window["width"];
			windowHeight = window["height"];
			printf("Loaded window config: %dx%d\n", windowWidth, windowHeight);
		}
	}
	else
	{
		printf("Could not load window config, using defaults: %dx%d\n", windowWidth, windowHeight);
	}
	// Initialize job system for background tasks
	if (!JobSystem::initialize())
	{
		printf("Warning: Failed to initialize job system\n");
	}
	// Create window with the configured size
	int options = CF_APP_OPTIONS_WINDOW_POS_CENTERED_BIT | CF_APP_OPTIONS_RESIZABLE_BIT;
	CF_Result result = make_app("Fancy Window Title", 0, 0, 0, windowWidth, windowHeight, options, argv[0]);
	cf_app_init_imgui();
	if (is_error(result))
		return -1;

	// Set up VFS for reading and writing (must be done after make_app)
	mount_content_directory_as("/assets");

	// Load window configuration again using VFS for viewport and debug windows
	DataFile windowConfig("/assets/window-config.json");

	// Read viewport dimensions from config (defaults to window size)
	float viewportWidth = (float)windowWidth;
	float viewportHeight = (float)windowHeight;
	bool debugHighlightViewport = false; // Default: don't highlight viewport

	if (windowConfig.contains("window"))
	{
		auto &window = windowConfig["window"];
		if (window.contains("viewportWidth") && window.contains("viewportHeight"))
		{
			viewportWidth = window["viewportWidth"];
			viewportHeight = window["viewportHeight"];
			printf("Loaded viewport config: %.0fx%.0f\n", viewportWidth, viewportHeight);
		}
		else
		{
			printf("No viewport size in config, using window size: %.0fx%.0f\n", viewportWidth, viewportHeight);
		}
	}

	// Read debug options from config
	bool debugHighlightNavmesh = false;					  // Default: don't highlight navmesh
	bool debughighlightNavMeshPaths = false;			  // Default: don't highlight navmesh points
	bool debugHighlightAgents = false;					  // Default: don't highlight agents
	bool debugHighlightCharacterHitboxes = false;		  // Default: don't show character hitboxes
	bool debugHighlightSpatialGrid = false;				  // Default: don't show spatial grid
	bool debugHighlightPlayerNavmeshCollisionBox = false; // Default: don't show player navmesh collision box
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
		if (debug.contains("highlightPlayerNavmeshCollisionBox"))
		{
			debugHighlightPlayerNavmeshCollisionBox = debug["highlightPlayerNavmeshCollisionBox"];
			printf("Debug highlightPlayerNavmeshCollisionBox: %s\n", debugHighlightPlayerNavmeshCollisionBox ? "enabled" : "disabled");
		}
	}

	// Create LevelV1 instance - handles all TMX and NavMesh initialization
	LevelV1 level("/assets/Levels/test_two");

	if (!level.isInitialized())
	{
		printf("Error: Failed to initialize level\n");
		destroy_app();
		return -1;
	}

	// Configure layer highlighting from config (parse once, use map for lookups)
	level.getLevelMap().setLayerHighlightConfig(windowConfig);

	// Get tile dimensions from level for proper spacing
	int tile_width = level.getTileWidth();
	int tile_height = level.getTileHeight();

	// Create debug window list and populate from config
	DebugWindowList debugWindows;

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
	bool showFPSMetrics = false;

	// Create Job system debug window if enabled in config
	std::unique_ptr<DebugJobWindow> jobWindow;
	bool showJobMetrics = false;

	// Player info debug window (created later after player is initialized)
	std::unique_ptr<DebugPlayerInfoWindow> playerInfoWindow;
	bool ShowPlayerInfo = false;

	if (windowConfig.contains("Debug"))
	{
		auto &debug = windowConfig["Debug"];

		if (debug.contains("ShowFPSMetrics"))
		{
			showFPSMetrics = debug["ShowFPSMetrics"];
			printf("Debug ShowFPSMetrics: %s\n", showFPSMetrics ? "enabled" : "disabled");

			if (showFPSMetrics)
			{
				fpsWindow = std::make_unique<DebugFPSWindow>("FPS Metrics");
				printf("Created FPS metrics debug window\n");
			}
		}

		if (debug.contains("ShowJobMetrics"))
		{
			showJobMetrics = debug["ShowJobMetrics"];
			printf("Debug ShowJobMetrics: %s\n", showJobMetrics ? "enabled" : "disabled");

			if (showJobMetrics)
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
	}

	// Create playerCharacter player character
	AnimatedDataCharacterNavMeshPlayer playerCharacter;

	// Starting position in tile coordinates (will be converted to world coordinates)
	float startTileX = 5.0f;
	float startTileY = 10.0f;

	// Convert tile coordinates to world pixel coordinates
	float startWorldX = startTileX * tile_width;
	float startWorldY = startTileY * tile_height;
	v2 playerPosition = cf_v2(startWorldX, startWorldY);

	printf("Player starting at tile (%.1f, %.1f) = world (%.1f, %.1f)\n",
		   startTileX, startTileY, startWorldX, startWorldY);

	if (!playerCharacter.init("assets/DataFiles/Entities/player.json"))
	{
		destroy_app();
		return -1;
	}

	// Set player's initial position
	playerCharacter.setPosition(playerPosition);

	// Connect player to level for hitbox collision detection
	playerCharacter.setLevel(&level);

	// Connect player to navmesh for walkable area detection
	playerCharacter.setNavMesh(&level.getNavMesh());

	// Set sprite dimensions for navmesh collision box calculation (tile size is the sprite size)
	playerCharacter.setSpriteDimensions(static_cast<float>(tile_width), static_cast<float>(tile_height));

	// Set initial hitbox visibility from config for player
	playerCharacter.sethitboxDebugActive(debugHighlightCharacterHitboxes);

	// Set hitbox visibility for all agents from config
	for (size_t i = 0; i < level.getAgentCount(); ++i)
	{
		AnimatedDataCharacterNavMeshAgent *agent = level.getAgent(i);
		if (agent)
		{
			agent->sethitboxDebugActive(debugHighlightCharacterHitboxes);
		}
	}

	// Create player info debug window if enabled (now that player and level are ready)
	if (ShowPlayerInfo)
	{
		playerInfoWindow = std::make_unique<DebugPlayerInfoWindow>("Player Info", playerCharacter, level);
		printf("Created Player info debug window\n");
	}

	// Create CF-native camera with explicit viewport dimensions from config
	CFNativeCamera cfCamera(cf_v2(0.0f, 0.0f), 1.0f, viewportWidth, viewportHeight);

	// Set up camera with basic settings
	cfCamera.setZoomRange(0.25f, 4.0f); // Allow 1/4x to 4x zoom

	// Make camera follow the player
	cfCamera.setTarget(&playerPosition);
	cfCamera.setFollowSpeed(3.0f);
	cfCamera.setFollowDeadzone(cf_v2(50.0f, 50.0f)); // 50px deadzone

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
	OnScreenChecks::initialize(&playerPosition, &cfCamera, &level);
	OnScreenChecks::start();

	// Main loop
	printf("Skeleton Adventure Game:\n");
	printf("  WASD - move playerCharacter\n");
	printf("  Q/E - camera zoom in/out\n");
	printf("  R - reset camera\n");
	printf("  T - test camera smooth movement\n");
	printf("  Y - test camera smooth zoom\n");
	printf("  U - test camera shake\n");
	printf("  1/2 - switch animations (idle/walk)\n");
	printf("  SPACE - reset playerCharacter position\n");
	printf("  N - toggle navmesh visualization\n");
	printf("  M - toggle navmesh points visualization\n");
	printf("  P - place/update navmesh point at player position\n");
	printf("  L - pathfind to navmesh point from player\n");
	printf("  ESC - quit\n");
	while (cf_app_is_running())
	{
		// Begin profiling the frame
		if (fpsWindow)
		{
			fpsWindow->beginFrame();
		}

		// Update app to handle window events and input (proper CF pattern)
		cf_app_update(NULL);

		// Handle ESC to quit
		if (cf_key_just_pressed(CF_KEY_ESCAPE))
		{
			break;
		}

		// Player movement (WASD) - only one direction at a time, most recent key takes priority
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

		// Handle playerCharacter animation input (1/2 for idle/walk)
		// playerCharacter.handleInput();

		// Handle spacebar to trigger action A
		if (cf_key_just_pressed(CF_KEY_SPACE))
		{
			Action *actionA = playerCharacter.getActionPointerA();
			if (actionA)
			{
				actionA->doAction();
				printf("Player triggered action A\n");
			}
		}

		// Camera feature demo keys
		if (cf_key_just_pressed(CF_KEY_T))
		{
			cfCamera.moveTo(cf_v2(playerPosition.x + 200.0f, playerPosition.y + 200.0f), 2.0f);
		}
		if (cf_key_just_pressed(CF_KEY_Y))
		{
			cfCamera.zoomTo(2.0f, 1.5f);
		}
		if (cf_key_just_pressed(CF_KEY_U))
		{
			cfCamera.shake(20.0f, 1.5f);
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
			if (level.getNavMesh().getPoint("player_marker") != nullptr)
			{
				level.getNavMesh().removePoint("player_marker");
			}
			// Add new point at player position
			level.getNavMesh().addPoint("player_marker", playerPosition);
			printf("NavMesh point placed at player position (%.1f, %.1f)\n", playerPosition.x, playerPosition.y);
		}

		// Camera zoom controls (Q/E) and reset (R)
		if (cf_key_just_pressed(CF_KEY_Q))
		{
			cfCamera.zoomOut(1.2f);
		}
		if (cf_key_just_pressed(CF_KEY_E))
		{
			cfCamera.zoomIn(1.2f);
		}
		if (cf_key_just_pressed(CF_KEY_R))
		{
			cfCamera.reset();
		}

		if (fpsWindow)
		{
			fpsWindow->markSection("Player Input");
		}
		level.updateAgents(dt);

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
		cfCamera.update(dt);

		if (fpsWindow)
		{
			fpsWindow->markSection("Camera Update");
		}
		// Render debug windows
		debugWindows.renderAll();

		if (fpsWindow)
		{
			fpsWindow->markSection("Debug Windows");
		}
		// Clear background
		CF_Color bg = make_color(0.1f, 0.1f, 0.15f, 1.0f);
		cf_draw_push_color(bg);
		cf_draw_quad_fill(make_aabb(v2(0, 0), (float)cf_app_get_width(), (float)cf_app_get_height()), 0.0f);
		cf_draw_pop_color();

		// Apply CF-native camera transformation for world-space rendering
		cfCamera.apply();

		// WORLD space drawing here (affected by camera)
		v2 text_position1 = cf_v2(0.0f, 0.0f); // World origin
		draw_text("Skeleton Adventure - TMX Level Map", text_position1);

		// 1. Render level tiles first
		level.renderLayers(cfCamera, windowConfig, 0.0f, 0.0f);

		// 2. Render all action hitboxes (player and agents) on top of tiles
		level.renderAgentActions(cfCamera, &playerCharacter);

		// 3. Render agents on top of action hitboxes
		level.renderAgents(cfCamera);

		if (fpsWindow)
		{
			fpsWindow->markSection("Level Render");
		}
		// Render NavMesh debug visualization (if enabled)
		if (showNavMesh && level.getNavMesh().getPolygonCount() > 0)
		{
			level.getNavMesh().debugRender(cfCamera);
		}

		// Render spatial grid debug visualization (if enabled)
		if (debugHighlightSpatialGrid && level.getSpatialGrid().getOccupiedCellCount() > 0)
		{
			level.getSpatialGrid().debugRender(cfCamera);
		}

		// Render NavMesh points debug visualization (if enabled)
		if (showNavMeshPoints && level.getNavMesh().getPointCount() > 0)
		{
			level.getNavMesh().debugRenderPoints(cfCamera);
		}

		// Render all NavMesh paths (if visualization is enabled)
		if (showNavMeshPoints && level.getNavMesh().getPathCount() > 0)
		{
			const auto &allPaths = level.getNavMesh().getPaths();
			for (const auto &path : allPaths)
			{
				if (path && path->isValid())
				{
					path->debugRender(cfCamera);
				}
			}
		}

		// Render agent position markers (if enabled)
		if (showAgents && level.getAgentCount() > 0)
		{
			cf_draw_push_color(cf_make_color_rgb(0, 255, 0)); // Green color

			for (size_t i = 0; i < level.getAgentCount(); ++i)
			{
				const AnimatedDataCharacterNavMeshAgent *agent = level.getAgent(i);
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

		// debug just highlight one tile
		highlightTile(level, 0, 0, cf_make_color_rgb(255, 0, 0));
		highlightTile(level, 10, 10, cf_make_color_rgb(255, 200, 0));

		//  Render playerCharacter at player position (world space)
		playerCharacter.render(playerPosition);

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
		cfCamera.restore();

		// UI space drawing (not affected by camera)
		// Get current window dimensions for proper UI positioning
		int current_width = cf_app_get_width();
		int current_height = cf_app_get_height();
		float top_y = -(current_height / 2.0f) + 20.0f; // 20px from top

		cfCamera.drawDebugInfo(10.0f, top_y);

		// Show player position in UI
		char playerInfo[256];
		snprintf(playerInfo, sizeof(playerInfo), "Player: (%.0f, %.0f)", playerPosition.x, playerPosition.y);
		draw_text(playerInfo, cf_v2(10.0f, top_y + 20.0f));

		// Draw viewport rectangle visualization (if enabled in config)
		if (debugHighlightViewport)
		{
			// Get viewport size from camera
			v2 viewport_size = cfCamera.getViewportSize();

			// Calculate viewport rectangle in screen space (centered)
			float half_vp_width = viewport_size.x / 2.0f;
			float half_vp_height = viewport_size.y / 2.0f;

			CF_Aabb viewport_rect = make_aabb(
				cf_v2(-half_vp_width, -half_vp_height),
				cf_v2(half_vp_width, half_vp_height));

			// Draw viewport boundary as a colored rectangle outline
			cf_draw_push_color(make_color(1.0f, 0.0f, 0.0f, 1.0f)); // Red
			cf_draw_quad(viewport_rect, 0.0f, 3.0f);				// 3px thick outline
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

		app_draw_onto_screen();
	}

	// Shutdown on-screen checks worker
	OnScreenChecks::requestShutdown();

	// Shutdown job system
	JobSystem::shutdown();

	// Cleanup on-screen checks
	OnScreenChecks::shutdown();

	destroy_app();
	return 0;
}

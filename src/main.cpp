#include <cute.h>
#include <stdio.h>
#include <cstdlib>
#include <dcimgui.h>
#include "lib/DebugWindow.h"
#include "lib/DataFileDebugWindow.h"
#include "lib/DebugWindowList.h"
#include "lib/Utils.h"
#include "lib/DataFile.h"
#include "lib/RealConfigFile.h"
#include "lib/tsx.h"
#include "lib/tmx.h"

#include "lib/CFNativeCamera.h"
#include "lib/SpriteAnimationDemo.h"
#include "lib/NavMesh.h"
#include "lib/NavMeshPath.h"
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
	bool debugHighlightNavmesh = false;		  // Default: don't highlight navmesh
	bool debugHighlightNavMeshPoints = false; // Default: don't highlight navmesh points
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
		if (debug.contains("highlightNavMeshPoints"))
		{
			debugHighlightNavMeshPoints = debug["highlightNavMeshPoints"];
			printf("Debug highlightNavMeshPoints: %s\n", debugHighlightNavMeshPoints ? "enabled" : "disabled");
		}
	}

	// Create TMX parser for the level
	// tmx levelMap("/assets/Levels/test_one/test_one.tmx");
	tmx levelMap("/assets/Levels/test_two/test_two.tmx");
	levelMap.debugPrint();

	// Configure layer highlighting from config (parse once, use map for lookups)
	levelMap.setLayerHighlightConfig(windowConfig);

	// Create NavMesh for pathfinding and collision detection
	NavMesh navmesh;
	// Try to build from a navmesh layer (looks in navmesh_layers first)
	// Common layer names: "navmesh", "nav_walkable", "collision", "walkable"
	// Set invert=true if empty tiles are walkable, false if filled tiles are walkable
	if (levelMap.getNavMeshLayerCount() > 0)
	{
		// Use the first navmesh layer found
		auto navLayer = levelMap.getNavMeshLayer(0);
		printf("Building navmesh from layer: %s\n", navLayer->name.c_str());
		navmesh.buildFromLayer(navLayer, levelMap.getTileWidth(), levelMap.getTileHeight(), 0.0f, 0.0f, false);
		printf("NavMesh created with %d polygons\n", navmesh.getPolygonCount());
	}
	else
	{
		printf("Warning: No navmesh layers found in level. Navigation mesh not created.\n");
	}

	// Get tile dimensions for proper spacing
	int tile_width = levelMap.getTileWidth();
	int tile_height = levelMap.getTileHeight();

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

	// Create skeleton player character
	SpriteAnimationDemo skeleton;
	v2 playerPosition = cf_v2(0.0f, 0.0f); // Start at world origin

	if (!skeleton.init())
	{
		destroy_app();
		return -1;
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
	bool showNavMeshPoints = debugHighlightNavMeshPoints;

	// NavMesh path for pathfinding
	NavMeshPath navmeshPath;

	// Main loop
	printf("Skeleton Adventure Game:\n");
	printf("  WASD - move skeleton\n");
	printf("  Q/E - camera zoom in/out\n");
	printf("  R - reset camera\n");
	printf("  T - test camera smooth movement\n");
	printf("  Y - test camera smooth zoom\n");
	printf("  U - test camera shake\n");
	printf("  1/2 - switch animations (idle/walk)\n");
	printf("  SPACE - reset skeleton position\n");
	printf("  N - toggle navmesh visualization\n");
	printf("  M - toggle navmesh points visualization\n");
	printf("  P - place/update navmesh point at player position\n");
	printf("  L - pathfind to navmesh point from player\n");
	printf("  ESC - quit\n");
	while (cf_app_is_running())
	{
		// Update app to handle window events and input (proper CF pattern)
		cf_app_update(NULL);

		// Handle ESC to quit
		if (cf_key_just_pressed(CF_KEY_ESCAPE))
		{
			break;
		}

		// Player movement (WASD)
		float dt = CF_DELTA_TIME;
		float playerSpeed = 200.0f; // pixels per second
		bool playerMoved = false;

		if (cf_key_down(CF_KEY_W) || cf_key_down(CF_KEY_UP))
		{
			playerPosition.y += playerSpeed * dt;
			playerMoved = true;
		}
		if (cf_key_down(CF_KEY_S) || cf_key_down(CF_KEY_DOWN))
		{
			playerPosition.y -= playerSpeed * dt;
			playerMoved = true;
		}
		if (cf_key_down(CF_KEY_A) || cf_key_down(CF_KEY_LEFT))
		{
			playerPosition.x -= playerSpeed * dt;
			playerMoved = true;
		}
		if (cf_key_down(CF_KEY_D) || cf_key_down(CF_KEY_RIGHT))
		{
			playerPosition.x += playerSpeed * dt;
			playerMoved = true;
		}

		// Handle skeleton animation input (1/2 for idle/walk)
		skeleton.handleInput();

		// Reset skeleton position
		if (cf_key_just_pressed(CF_KEY_SPACE))
		{
			playerPosition = cf_v2(0.0f, 0.0f);
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
			if (navmesh.getPoint("player_marker") != nullptr)
			{
				navmesh.removePoint("player_marker");
			}
			// Add new point at player position
			navmesh.addPoint("player_marker", playerPosition);
			printf("NavMesh point placed at player position (%.1f, %.1f)\n", playerPosition.x, playerPosition.y);
		}

		// Pathfind to NavMesh point from player position
		if (cf_key_just_pressed(CF_KEY_L))
		{
			// Check if there's a player_marker point to pathfind to
			const NavMeshPoint *targetPoint = navmesh.getPoint("player_marker");
			if (targetPoint != nullptr)
			{
				printf("Attempting to pathfind from player (%.1f, %.1f) to marker (%.1f, %.1f)\n",
					   playerPosition.x, playerPosition.y, targetPoint->position.x, targetPoint->position.y);

				if (navmeshPath.generateToPoint(navmesh, playerPosition, "player_marker"))
				{
					printf("Path generated successfully with %d waypoints\n", navmeshPath.getWaypointCount());
				}
				else
				{
					printf("Failed to generate path\n");
				}
			}
			else
			{
				printf("No 'player_marker' point found. Press P to place a marker first.\n");
			}
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

		// Update skeleton animation
		skeleton.update(dt);

		// Update camera (handles following and smooth movement)
		cfCamera.update(dt);

		// Render debug windows
		debugWindows.renderAll();

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

		// Render TMX level (with CF-native camera transformations and config for layer highlighting)
		levelMap.renderAllLayers(cfCamera, windowConfig, 0.0f, 0.0f);

		// Render NavMesh debug visualization (if enabled)
		if (showNavMesh && navmesh.getPolygonCount() > 0)
		{
			navmesh.debugRender(cfCamera);
		}

		// Render NavMesh points debug visualization (if enabled)
		if (showNavMeshPoints && navmesh.getPointCount() > 0)
		{
			navmesh.debugRenderPoints(cfCamera);
		}

		// Render NavMesh path (if valid and points visualization is enabled)
		if (showNavMeshPoints && navmeshPath.isValid())
		{
			navmeshPath.debugRender(cfCamera);
		}

		// Render skeleton at player position (world space)
		skeleton.render(playerPosition);

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

		app_draw_onto_screen();
	}

	destroy_app();
	return 0;
}

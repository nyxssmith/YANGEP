#include <cute.h>
#include <stdio.h>
#include <cstdlib>
#include <dcimgui.h>
#include "lib/DebugWindow.h"
#include "lib/DataFileDebugWindow.h"
#include "lib/Utils.h"
#include "lib/DataFile.h"
#include "lib/tsx.h"
#include "lib/tmx.h"

#include "lib/CFNativeCamera.h"
#include "lib/SpriteAnimationDemo.h"
using namespace Cute;

int main(int argc, char *argv[])
{
	// Create a window with a resolution of 640 x 480.
	int options = CF_APP_OPTIONS_WINDOW_POS_CENTERED_BIT | CF_APP_OPTIONS_RESIZABLE_BIT;
	CF_Result result = make_app("Fancy Window Title", 0, 0, 0, 640, 480, options, argv[0]);
	cf_app_init_imgui();
	if (is_error(result))
		return -1;

	// Set up VFS for reading and writing
	mount_content_directory_as("/assets");

	// Load game data
	DataFile df("assets/DataFiles/EntityFiles/skeleton.json");

	// Create TMX parser for the level
	tmx levelMap("/assets/Levels/test_one/test_one.tmx");
	levelMap.debugPrint();

	// Get tile dimensions for proper spacing
	int tile_width = levelMap.getTileWidth();
	int tile_height = levelMap.getTileHeight();

	// Create debug windows
	DebugWindow debugWindow("Debug Info");
	DataFileDebugWindow dataFileDebugWindow("DataFile Viewer", df);

	// Create skeleton player character
	SpriteAnimationDemo skeleton;
	v2 playerPosition = cf_v2(0.0f, 0.0f); // Start at world origin

	if (!skeleton.init()) {
		destroy_app();
		return -1;
	}

	// Create CF-native camera for handling view transformations
	CFNativeCamera cfCamera(cf_v2(0.0f, 0.0f), 1.0f); // Start at origin with normal zoom

	// Set up camera with basic settings
	cfCamera.setZoomRange(0.25f, 4.0f);			   // Allow 1/4x to 4x zoom

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
	printf("  ESC - quit\n");
	while (cf_app_is_running()) {
		// Update app to handle window events and input (proper CF pattern)
		cf_app_update(NULL);

		// Handle ESC to quit
		if (cf_key_just_pressed(CF_KEY_ESCAPE)) {
			break;
		}

		// Player movement (WASD)
		float dt = CF_DELTA_TIME;
		float playerSpeed = 200.0f; // pixels per second
		bool playerMoved = false;

		if (cf_key_down(CF_KEY_W) || cf_key_down(CF_KEY_UP)) {
			playerPosition.y += playerSpeed * dt;
			playerMoved = true;
		}
		if (cf_key_down(CF_KEY_S) || cf_key_down(CF_KEY_DOWN)) {
			playerPosition.y -= playerSpeed * dt;
			playerMoved = true;
		}
		if (cf_key_down(CF_KEY_A) || cf_key_down(CF_KEY_LEFT)) {
			playerPosition.x -= playerSpeed * dt;
			playerMoved = true;
		}
		if (cf_key_down(CF_KEY_D) || cf_key_down(CF_KEY_RIGHT)) {
			playerPosition.x += playerSpeed * dt;
			playerMoved = true;
		}

		// Handle skeleton animation input (1/2 for idle/walk)
		skeleton.handleInput();

		// Reset skeleton position
		if (cf_key_just_pressed(CF_KEY_SPACE)) {
			playerPosition = cf_v2(0.0f, 0.0f);
		}

		// Camera feature demo keys
		if (cf_key_just_pressed(CF_KEY_T)) {
			cfCamera.moveTo(cf_v2(playerPosition.x + 200.0f, playerPosition.y + 200.0f), 2.0f);
		}
		if (cf_key_just_pressed(CF_KEY_Y)) {
			cfCamera.zoomTo(2.0f, 1.5f);
		}
		if (cf_key_just_pressed(CF_KEY_U)) {
			cfCamera.shake(20.0f, 1.5f);
		}

		// Camera zoom controls (Q/E) and reset (R)
		if (cf_key_just_pressed(CF_KEY_Q)) {
			cfCamera.zoomOut(1.2f);
		}
		if (cf_key_just_pressed(CF_KEY_E)) {
			cfCamera.zoomIn(1.2f);
		}
		if (cf_key_just_pressed(CF_KEY_R)) {
			cfCamera.reset();
		}

		// Update skeleton animation
		skeleton.update(dt);

		// Update camera (handles following and smooth movement)
		cfCamera.update(dt);

		// Render debug windows
		debugWindow.render();
		dataFileDebugWindow.render();

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

		// Render TMX level (with CF-native camera transformations and culling)
		levelMap.renderAllLayers(cfCamera, 0.0f, 0.0f);

		// Render skeleton at player position (world space)
		skeleton.render(playerPosition);

		// Restore camera transformation
		cfCamera.restore();

		// UI space drawing (not affected by camera)
		cfCamera.drawDebugInfo(10.0f, -240.0f + 20.0f); // Top-left corner

		// Show player position in UI
		char playerInfo[256];
		snprintf(playerInfo, sizeof(playerInfo), "Player: (%.0f, %.0f)", playerPosition.x, playerPosition.y);
		draw_text(playerInfo, cf_v2(10.0f, -240.0f + 40.0f));

		app_draw_onto_screen();
	}

	destroy_app();
	return 0;
}

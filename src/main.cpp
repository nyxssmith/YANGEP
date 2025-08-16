#include <cute.h>
#include <stdio.h>
#include <cstdlib>
#include <cimgui.h>
#include "lib/DebugWindow.h"
#include "lib/DataFileDebugWindow.h"
#include "lib/Utils.h"
#include "lib/DataFile.h"
#include "lib/SpriteDemo.h"
#include "lib/tsx.h"
#include "lib/tmx.h"
#include "lib/Camera.h"
using namespace Cute;

int main(int argc, char *argv[])
{
	// Create a window with a resolution of 640 x 480.
	int options = CF_APP_OPTIONS_WINDOW_POS_CENTERED_BIT | CF_APP_OPTIONS_RESIZABLE_BIT;
	CF_Result result = make_app("Fancy Window Title", 0, 0, 0, 640, 480, options, argv[0]);
	cf_app_init_imgui();
	if (is_error(result))
		return -1;

	// Mount the assets directory for file I/O
	mount_content_directory_as("/assets");

	// read datafile
	// DataFile df("/assets/a.json");
	DataFile df("assets/DataFiles/EntityFiles/skeleton.json");
	// printf("DataFile data: %s\n", df.dump(4).c_str());

	// Create TMX parser for the level
	tmx levelMap("/assets/Levels/test_one/test_one.tmx");

	// Print debug information about the TMX file
	levelMap.debugPrint();
	/*
	// For comparison, also load individual TSX parser
	tsx levelTxs("/assets/Art/LevelPNGs/magecity.tsx");

	// Get some individual tile sprites to render for comparison
	CF_Sprite tile_0_0 = levelTxs.getTile(0, 0); // First tile
	CF_Sprite tile_1_0 = levelTxs.getTile(1, 0); // Second tile in first row
	CF_Sprite tile_0_1 = levelTxs.getTile(0, 1); // First tile in second row
	CF_Sprite tile_2_2 = levelTxs.getTile(2, 2); // Third tile in third row
	*/
	// Get tile dimensions for proper spacing
	int tile_width = levelMap.getTileWidth();
	int tile_height = levelMap.getTileHeight();

	printf("Using tile dimensions: %dx%d pixels\n", tile_width, tile_height);

	// Create debug window
	DebugWindow debugWindow("Debug Info aaaa");
	DataFileDebugWindow dataFileDebugWindow("DataFile Viewer", df);

	// Create camera for handling view transformations
	Camera camera(cf_v2(0.0f, 0.0f), 1.0f); // Start at origin with normal zoom

	// Set up camera with some nice settings
	camera.setZoomRange(0.25f, 4.0f);			   // Allow 1/4x to 4x zoom
	camera.setFollowSpeed(2.0f);				   // Smooth following
	camera.setFollowDeadzone(cf_v2(50.0f, 50.0f)); // Deadzone around target

	while (app_is_running())
	{
		app_update();

		// Handle camera input (WASD for movement, mouse wheel for zoom)
		float camera_speed = 200.0f; // pixels per second
		float dt = CF_DELTA_TIME;

		if (key_down(CF_KEY_S) || key_down(CF_KEY_DOWN))
		{
			camera.translate(0.0f, -camera_speed * dt);
		}
		if (key_down(CF_KEY_W) || key_down(CF_KEY_UP))
		{
			camera.translate(0.0f, camera_speed * dt);
		}
		if (key_down(CF_KEY_A) || key_down(CF_KEY_LEFT))
		{
			camera.translate(-camera_speed * dt, 0.0f);
		}
		if (key_down(CF_KEY_D) || key_down(CF_KEY_RIGHT))
		{
			camera.translate(camera_speed * dt, 0.0f);
		}

		// Zoom with Q/E keys
		if (key_just_pressed(CF_KEY_Q))
		{
			camera.zoomOut(1.2f);
		}
		if (key_just_pressed(CF_KEY_E))
		{
			camera.zoomIn(1.2f);
		}

		// Reset camera with R key
		if (key_just_pressed(CF_KEY_R))
		{
			camera.reset();
		}

		// Camera shake with SPACE
		if (key_just_pressed(CF_KEY_SPACE))
		{
			camera.shake(20.0f, 0.5f);
		}

		// Update camera
		camera.update(dt);

		// Render debug windows
		// debugWindow.render();
		// dataFileDebugWindow.render();

		// Apply camera transformation for world-space rendering
		camera.apply();

		// Draw debug objects in world space
		// Main reference square at origin
		draw_quad(CF_Aabb{cf_v2(-50.0f, -50.0f), cf_v2(50.0f, 50.0f)}, 2.0f, 0.0f);

		// Grid of smaller squares to show camera movement
		for (int x = -5; x <= 5; x++)
		{
			for (int y = -5; y <= 5; y++)
			{
				if (x == 0 && y == 0)
					continue; // Skip the center square

				float square_x = x * 150.0f;
				float square_y = y * 150.0f;
				CF_Color color = cf_color_blue();

				// Make some squares different colors for reference
				if (x == 0 || y == 0)
					color = cf_color_green(); // Axis squares
				if (std::abs(x) == 1 && std::abs(y) == 1)
					color = cf_color_yellow(); // Corner squares

				draw_quad(CF_Aabb{cf_v2(square_x - 25.0f, square_y - 25.0f),
								  cf_v2(square_x + 25.0f, square_y + 25.0f)},
						  2.0f, 0.0f);
			}
		}

		// Draw coordinate axes
		// X-axis (horizontal red line)
		draw_line(cf_v2(-4000.0f, 0.0f), cf_v2(4000.0f, 0.0f), 3.0f);
		// Y-axis (vertical green line)
		draw_line(cf_v2(0.0f, -4000.0f), cf_v2(0.0f, 4000.0f), 3.0f);

		// Restore camera transformation
		camera.restore();

		// Draw UI elements (not affected by camera)
		// Position text at the bottom in screen space
		v2 text_position1 = cf_v2(-200, 250);
		draw_text("TMX Level Map (Camera Controlled)", text_position1);

		// Draw camera debug info
		camera.drawDebugInfo();

		app_draw_onto_screen(true);
	}

	destroy_app();
	return 0;
}

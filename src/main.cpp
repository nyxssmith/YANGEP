#include <cute.h>
#include <stdio.h>
#include <cimgui.h>
#include "lib/DebugWindow.h"
#include "lib/DataFileDebugWindow.h"
#include "lib/Utils.h"
#include "lib/DataFile.h"
#include "lib/SpriteDemo.h"
#include "lib/tsx.h"
#include "lib/tmx.h"
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

	while (app_is_running())
	{
		app_update();

		// Render debug windows
		// debugWindow.render();
		// dataFileDebugWindow.render();

		// Render the TMX map layers
		// Position the map at top-left area of screen
		// Coordinate system: (0,0) is top-left, +X goes right, +Y goes down
		float map_x = -300.0f; // Left side of screen
		float map_y = -200.0f; // Near top of screen

		// Render all layers of the TMX map
		levelMap.renderAllLayers(map_x, map_y);

		// Demonstrate coordinate system with some debug info
		// Show what tile is at map coordinates (0,0), (1,0), (0,1)
		static bool debug_printed = false;
		if (!debug_printed)
		{
			printf("\n=== Coordinate System Demo ===\n");
			printf("Map positioned at world coordinates (%.1f, %.1f)\n", map_x, map_y);

			// Calculate world positions for some example tiles
			float tile_world_x, tile_world_y;
			levelMap.mapToWorldCoords(0, 0, map_x, map_y, tile_world_x, tile_world_y);
			printf("Map tile (0,0) -> World position (%.1f, %.1f)\n", tile_world_x, tile_world_y);

			levelMap.mapToWorldCoords(1, 0, map_x, map_y, tile_world_x, tile_world_y);
			printf("Map tile (1,0) -> World position (%.1f, %.1f)\n", tile_world_x, tile_world_y);

			levelMap.mapToWorldCoords(0, 1, map_x, map_y, tile_world_x, tile_world_y);
			printf("Map tile (0,1) -> World position (%.1f, %.1f)\n", tile_world_x, tile_world_y);

			printf("Coordinate system: (0,0) = top-left, +X = right, +Y = down\n");
			printf("==============================\n\n");
			debug_printed = true;
		}

		// Position text at the bottom
		v2 text_position1 = cf_v2(-200, 250);

		// Draw the text
		draw_text("TMX Level Map (Left)", text_position1);

		app_draw_onto_screen(true);
	}

	destroy_app();
	return 0;
}

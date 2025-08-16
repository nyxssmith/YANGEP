#include <cute.h>
#include <stdio.h>
#include <cimgui.h>
#include "lib/DebugWindow.h"
#include "lib/DataFileDebugWindow.h"
#include "lib/Utils.h"
#include "lib/DataFile.h"
#include "lib/SpriteDemo.h"
#include "lib/tsx.h"
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

	// Create TSX parser
	tsx levelTxs("/assets/Art/LevelPNGs/magecity.tsx");

	// Print debug information about the TSX file
	// levelTxs.debugPrint();

	// Get some tile sprites to render
	CF_Sprite tile_0_0 = levelTxs.getTile(0, 0); // First tile
	CF_Sprite tile_1_0 = levelTxs.getTile(1, 0); // Second tile in first row
	CF_Sprite tile_0_1 = levelTxs.getTile(0, 1); // First tile in second row
	CF_Sprite tile_2_2 = levelTxs.getTile(2, 2); // Third tile in third row

	// Get tile dimensions for proper spacing
	int tile_width = levelTxs.getTileWidth();
	int tile_height = levelTxs.getTileHeight();

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

		// Render tiles from the TSX tileset with proper spacing
		// Define starting position for the tile grid
		int start_x = -200;
		int start_y = -150;

		// Draw tile (0,0) at starting position
		cf_draw_push();
		cf_draw_translate_v2(cf_v2(start_x, start_y));
		cf_draw_sprite(&tile_0_0);
		cf_draw_pop();

		// Draw tile (1,0) - one tile width to the right
		cf_draw_push();
		cf_draw_translate_v2(cf_v2(start_x + tile_width, start_y));
		cf_draw_sprite(&tile_1_0);
		cf_draw_pop();

		// Draw tile (0,1) - one tile height below
		cf_draw_push();
		cf_draw_translate_v2(cf_v2(start_x, start_y + tile_height));
		cf_draw_sprite(&tile_0_1);
		cf_draw_pop();

		// Draw tile (2,2) - two tile widths right and two tile heights down
		cf_draw_push();
		cf_draw_translate_v2(cf_v2(start_x + 2 * tile_width, start_y + 2 * tile_height));
		cf_draw_sprite(&tile_2_2);
		cf_draw_pop();

		// Position text below the tiles
		v2 text_position = cf_v2(0, 100);

		// Draw the text
		draw_text("TSX Tileset Rendering Demo", text_position);

		app_draw_onto_screen(true);
	}

	destroy_app();
	return 0;
}

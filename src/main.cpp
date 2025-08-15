#include <cute.h>
#include <stdio.h>
#include <cimgui.h>
#include "lib/DebugWindow.h"
#include "lib/DataFileDebugWindow.h"
#include "lib/Utils.h"
#include "lib/DataFile.h"
#include "lib/SpriteDemo.h"
using namespace Cute;

int main(int argc, char *argv[])
{
	// Create a window with a resolution of 640 x 480.
	int options = CF_APP_OPTIONS_WINDOW_POS_CENTERED_BIT;
	CF_Result result = make_app("Fancy Window Title", 0, 0, 0, 640, 480, options, argv[0]);
	cf_app_init_imgui();
	if (is_error(result))
		return -1;

	// Mount the assets directory for file I/O
	mount_content_directory_as("/assets");

	// read datafile
	DataFile df("/assets/a.json");
	printf("DataFile data: %s\n", df.dump(4).c_str());

	// Create debug window
	DebugWindow debugWindow("Debug Info aaaa");
	DataFileDebugWindow dataFileDebugWindow("DataFile Viewer", df);

	// Create sprite demo
	SpriteDemo spriteDemo;
	if (!spriteDemo.initialize())
	{
		printf("Failed to initialize sprite demo\n");
		return -1; // Exit if sprite demo fails to initialize
	}

	while (app_is_running())
	{
		app_update();

		// Render debug windows
		debugWindow.render();
		dataFileDebugWindow.render();
		// Update sprite demo
		spriteDemo.update(2.116f); // ~60 FPS

		// Position text below the triangles (negative Y to go down)
		v2 text_position = v2(0, 100); // Below the triangles

		// Draw the text
		draw_text("my girlfriend is so pretty", text_position);

		// Render sprite demo
		spriteDemo.render();

		app_draw_onto_screen(true);
	}

	destroy_app();
	return 0;
}

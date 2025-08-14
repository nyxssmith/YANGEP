#include <cute.h>
#include <stdio.h>
#include <cimgui.h>
#include "lib/DebugWindow.h"
#include "lib/Utils.h"
#include "lib/DataFile.h"
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

	// Load JSON data
	nlohmann::json json_data = ReadJson("/assets/a.json");
	printf("JSON data: %s\n", json_data.dump(4).c_str());
	DataFile df("/assets/a.json");
	printf("DataFile data: %s\n", df.dump(4).c_str());
	// Create debug window
	DebugWindow debugWindow("Debug Info aaaa");

	while (app_is_running())
	{
		app_update();

		// Render debug window
		debugWindow.render();
		// Position text below the triangles (negative Y to go down)
		v2 text_position = v2(0, 100); // Below the triangles

		// Draw the text
		draw_text("my girlfriend is so pretty", text_position);

		app_draw_onto_screen(true);
	}

	destroy_app();
	return 0;
}

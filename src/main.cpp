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

	// shared scene setup
	SceneManager sceneManager;
	sceneManager.SharedSetup(windowWidth, windowHeight);
	// Main loop
	// set first scene
	sceneManager.LoadScene("level_one");
	bool isRunning = true;
	while (isRunning && cf_app_is_running())
	{
		// main loop
		isRunning = sceneManager.MainLoop();
		// TODO return struct of either SHOW_LOADING_SCREEN, LEVEL_RENDERED, MENU_RENDERED, SHUTDOWN
		//  to optionally handle loading screens that arent instant
	}
	// cleanup shared resources
	sceneManager.SharedCleanup();

	destroy_app();
	return 0;
}

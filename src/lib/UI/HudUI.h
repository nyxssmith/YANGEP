#ifndef HUD_UI_H
#define HUD_UI_H

#include <cute.h>
#include <vector>
#include <string>
#include <optional>

class Inventory;

// Simple HUD/UI renderer using Dear ImGui.
class HudUI
{
public:
	struct Icon
	{
		// Optional texture path for future asset hookup.
		std::string texturePath;
		// Logical state flags.
		bool active = false;
		bool disabled = false;
		// Optional tooltip text.
		std::string tooltip;
	};

	// Initialize (placeholder - no hard asset dependency).
	void initialize();
	// Render left column of icons.
	void renderLeftColumn(const std::vector<Icon>& icons, float iconSize = 48.0f, float padding = 6.0f);
	// Render bottom row of icons.
	void renderBottomRow(const std::vector<Icon>& icons, float iconSize = 48.0f, float padding = 8.0f);
	// Render Inventory window with edge/background art (placeholder if textures missing).
	void renderInventoryWindow(const Inventory* inventory, bool* open, int cols = 4, float slotSize = 48.0f, float slotPad = 6.0f);
};

#endif // HUD_UI_H


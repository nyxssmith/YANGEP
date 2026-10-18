#ifndef HUD_UI_H
#define HUD_UI_H

#include <cute.h>
#include <vector>
#include <string>
#include <optional>

class Inventory;
class Item;

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
	// Render top row of heart and shield icons; icons swap to the *_missing variant when current < max.
	void renderTopRow(float padding, float icon_size, int max_health, int current_health, int max_shields, int current_shields);
	// Render left column of icons.
	void renderLeftColumn(const std::vector<Icon> &icons, float iconSize = 48.0f, float padding = 6.0f);
	// Render bottom row of icons.
	void renderBottomRow(float padding, float iconSize = 48.0f, Item *leftClawItem = nullptr, Item *rightClawItem = nullptr);
	// Render Inventory window with edge/background art (placeholder if textures missing).
	void renderInventoryWindow(const Inventory *inventory, bool *open, int cols = 4, float slotSize = 48.0f, float slotPad = 6.0f);
};

#endif // HUD_UI_H

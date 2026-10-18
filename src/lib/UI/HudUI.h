#ifndef HUD_UI_H
#define HUD_UI_H

#include <cute.h>
#include <vector>
#include <string>
#include <unordered_map>

class Inventory;
class Item;

struct HudAtlasRect
{
  int x = 0;
  int y = 0;
  int w = 0;
  int h = 0;
};

// Simple HUD/UI renderer using Dear ImGui.
class HudUI
{
public:
  struct Icon
  {
    std::string texturePath;
    bool active = false;
    bool disabled = false;
    std::string tooltip;
  };

  void initialize();
  void renderTopRow(float padding, float icon_size, int max_health, int current_health, int max_shields, int current_shields);
  void renderLeftColumn(const std::vector<Icon> &icons, float iconSize = 48.0f, float padding = 6.0f);
  void renderBottomRow(float padding, float iconSize = 48.0f, Item *leftClawItem = nullptr, Item *rightClawItem = nullptr);
  void renderInventoryWindow(const Inventory *inventory, bool *open, int cols = 4, float slotSize = 48.0f, float slotPad = 6.0f);

  // Use void* (ImTextureID representation) in the header to avoid any type/typedef header ordering conflicts
  static bool get_texture(const std::string &path, void *&out);
};

#endif // HUD_UI_H
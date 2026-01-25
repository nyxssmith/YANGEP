# HUD and Inventory UI (Dear ImGui + GUI assets)

Status: Planned/Active (Assets in progress)
Owner: UI/Gameplay

## Overview
Implement an in‑game HUD using Dear ImGui plus a library of GUI images. The HUD provides:

- Left‑side vertical icon column (stateful image buttons)
- Bottom action bar (stateful image buttons)
- Center screen Inventory window (using `Inventory`, with edge‑art and a textured background)

This feature is non‑blocking to core gameplay and can be hidden when not focused.

## Requirements
- Dear ImGui is already integrated via Cute Framework helpers (ImGui backend init in `main.cpp`).
- GUI textures (PNG) for icons (active/disabled/cooldown), inventory window edge art, and a background tile/texture.
- Non‑blocking input toggles:
  - I: Toggle Inventory window
  - Esc/Game state: close inventory when appropriate

## UI Elements

### 1) Left Column Icons (image buttons)
- Renders a fixed vertical strip along the left edge.
- Each slot is an image button; shows state:
  - Active: normal color
  - Disabled: dimmed/greyscale tint
  - Cooldown: overlay radial/alpha bar (optional follow‑up)
- Hover tooltip shows a short description/hotkey.
- Layout constants:
  - Icon size: 48 px (configurable)
  - Padding: 6 px between icons
  - Margin from screen edge: 8–12 px

### 2) Bottom Row Action Bar (image buttons)
- Renders a horizontal row along the bottom center/edge.
- Per‑icon state same as left column.
- Optional hotkey labels (e.g., 1,2,3,4…).
- Layout constants:
  - Icon size: 48 px (configurable)
  - Padding: 8 px
  - Bar margin from screen edge: 8–12 px

### 3) Inventory Window (center)
- Uses `Inventory` (`src/lib/Items/Inventory.cpp`) to render slots grid.
- Visuals:
  - Edge art: 9‑slice or tiled borders drawn behind ImGui content.
  - Background: textured panel (repeat/tile or stretch; choose a consistent style).
- Contents:
  - Grid with N columns (default 4), slot size 48 px.
  - Empty slot: placeholder frame.
  - Occupied slot: item icon + optional name tooltip.
- Controls:
  - I to open/close.
  - Clicking an item selects it; right‑click opens context (drop/use) (follow‑up).

## Data & State
- Icon state should be driven by gameplay data:
  - Player status flags, cooldown timers, action availability.
- Inventory reads from the active `Inventory` object (capacity, items per slot) without mutating on display.

## Integration Plan
1. Add `HudUI` module:
   - `src/lib/UI/HudUI.h/.cpp` provides functions:
     - `renderLeftColumn(icons, iconSize, padding)`
     - `renderBottomRow(icons, iconSize, padding)`
     - `renderInventoryWindow(inventory, &open, cols, slotSize, slotPad)`
2. Asset hookup:
   - Load icon textures and inventory panel textures; keep ImGui texture IDs (ImTextureID) cached.
3. Input:
   - Add toggle key (I) in `main.cpp` to show/hide inventory window.
4. Rendering order:
   - After gameplay and debug windows, call `HudUI::render...` so HUD overlays the scene.
5. State wiring:
   - Provide a lightweight adapter that maps game state → `Icon` structs (active/disabled/cooldown).

## Progress Update (2026-01-24)
- GUI atlas source PNG loaded and labeled via AtlasLabeler.
- Autocut JSON regenerated from `RPG_GUI_moonmod_source.png`.
- Verified labeled items include window borders and icon background frames.
- Added `paper_background.png` as inventory window background candidate.
- AtlasLabeler now shows index labels on green boxes for faster naming.

## Asset Mapping (Current)
- Inventory window frame: `border_double_*` pieces (corners, edges, intersections).
- Inventory background: `paper_background.png`.
- Icon frames: `icon_background_small` / `icon_background_large`.
- Optional controls: `plus/minus_arrow_*`, `checkbox_*`, `radio_button_*`.

## Textures & Sizing
- Icon textures: 48×48 px (or power‑of‑two atlas).
- Panel textures: edge art for 9‑slice (corners/edges/center), or a large baked background.
- Theme: consistent color grade to match game palette.

## Open Questions
- Do we support drag‑and‑drop between inventory slots in phase 1?
--- not building a game that requires a mouse, will need to make some key bindings to activate actions on an item (e.g. Use, Equip, Drop, Describe)
- Should cooldown appear as radial mask or progress bar overlay?
- Do we block game input while the inventory is open?
- Do we need dedicated art for inventory slot frames vs reusing icon backgrounds?
- Are there labeled icons for status/action imagery or should we label more atlas items?

## Follow‑ups
- Radial cooldown shader overlay for icons.
- Drag‑and‑drop and context menus for inventory.
- Persist HUD/UI settings (scale, visibility) in config.

## References
- Inventory source: `src/lib/Items/Inventory.cpp`
- Main loop (integration point): `src/main.cpp`
- HUD module (to be added): `src/lib/UI/HudUI.h/.cpp`


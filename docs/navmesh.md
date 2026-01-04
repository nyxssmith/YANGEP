# NavMesh System Documentation

## Overview

The NavMesh (Navigation Mesh) system provides pathfinding and navigation for characters in the game. It is built from TMX tile map layers and supports various special layer types for different navigation features.

## Layer Types and Naming Conventions

The NavMesh system uses **case-insensitive prefix matching** to categorize layers based on their names. This allows for flexible naming while maintaining clear categorization.

### NavMesh Layers

**Prefix:** `navmesh`, `nav_`

**Purpose:** Define the walkable areas of the level.

**Examples:**

- `navmesh`
- `navmesh_floor`
- `nav_ground`
- `NavMesh_Level1`

**Behavior:**

- Tiles with GID ≠ 0 are considered walkable by default
- Can be inverted (empty tiles = walkable) via the `invert` parameter
- Creates rectangular polygons for each walkable tile
- Automatically calculates neighbor relationships between adjacent tiles

### Structure Layers

**Prefix:** `structure`, `structure_`, `structure `

**Purpose:** Define multi-level structures that are rendered separately from regular layers.

**Examples:**

- `structure`
- `structure_buildings`
- `structure_walls`
- `Structure Layer 1`

**Behavior:**

- Rendered with world Y position sorting for proper layering
- Cached with lowest world Y coordinate for efficient rendering
- Not included in regular layer rendering

### Cut Layers (NavMesh Edge Blocking)

Cut layers define edges that block navigation, even between walkable tiles. Each cut layer type blocks movement across a specific edge direction.

#### Bottom Edge Cuts

**Prefix:** `cutb`

**Examples:**

- `cutb`
- `cutb_fences`
- `cutbottom`

**Behavior:**

- Tiles with GID ≠ 0 create a cut on the **bottom edge** (TMX coordinates: Y+1 direction)
- Blocks navigation from the tile to the tile below it
- Removes neighbor relationship between the tile and its bottom neighbor

#### Top Edge Cuts

**Prefix:** `cutt`

**Examples:**

- `cutt`
- `cutt_barriers`
- `cuttop`

**Behavior:**

- Tiles with GID ≠ 0 create a cut on the **top edge** (TMX coordinates: Y-1 direction)
- Blocks navigation from the tile to the tile above it
- Removes neighbor relationship between the tile and its top neighbor

#### Right Edge Cuts

**Prefix:** `cutr`

**Examples:**

- `cutr`
- `cutr_walls`
- `cutright`

**Behavior:**

- Tiles with GID ≠ 0 create a cut on the **right edge** (TMX coordinates: X+1 direction)
- Blocks navigation from the tile to the tile to the right
- Removes neighbor relationship between the tile and its right neighbor

#### Left Edge Cuts

**Prefix:** `cutl`

**Examples:**

- `cutl`
- `cutl_doors`
- `cutleft`

**Behavior:**

- Tiles with GID ≠ 0 create a cut on the **left edge** (TMX coordinates: X-1 direction)
- Blocks navigation from the tile to the tile to the left
- Removes neighbor relationship between the tile and its left neighbor

## TMX Coordinate System

The NavMesh system uses TMX coordinate conventions:

- **Origin:** (0, 0) is at the **top-left**
- **X-axis:** Increases to the **right**
- **Y-axis:** Increases **downward**

This is automatically converted to the game's rendering coordinate system where Y increases upward.

## Edge Direction Reference

```
        Top (Y-1)
           |
   Left ---+--- Right
   (X-1)   |   (X+1)
           |
      Bottom (Y+1)
```

## NavMesh Generation Process

1. **Layer Loading**

   - TMX layers are parsed and categorized by name prefix
   - NavMesh layers, structure layers, and cut layers are separated

2. **Mesh Building**

   - One rectangular polygon is created per walkable tile
   - Neighbor relationships are calculated between adjacent polygons
   - Edges are created for each polygon side

3. **Cut Application**

   - After mesh building, cut layers are processed
   - For each marked tile in a cut layer, the corresponding edge's neighbor relationship is removed
   - This creates impassable boundaries within the walkable area

4. **Pathfinding Setup**
   - A\* pathfinding algorithm uses the polygon neighbor graph
   - Paths cannot cross edges where neighbor relationships have been removed

## Edge Rendering (Debug Visualization)

The NavMesh renders edges in different colors to visualize the navigation structure:

- **Purple edges:** Boundary or cut edges (no neighbor on one or both sides)
  - Mesh perimeter edges
  - Edges blocked by cut layers
- **Red edges:** Internal edges (connecting two walkable polygons)
  - Normal passable edges between neighbors

## Movement Collision Detection

Characters check two conditions when moving:

1. **Walkable Check:** Is the destination position inside a walkable polygon?
2. **Edge Crossing Check:** Does the movement path cross any boundary edge?

This ensures characters cannot:

- Walk off the navmesh
- Cross cut edges even if both start and end positions are walkable

## Usage Example

### Creating a Level with NavMesh and Cuts

1. **Create NavMesh Layer:**

   ```
   Layer name: "navmesh_floor"
   Paint tiles where characters can walk
   ```

2. **Create Cut Layers for Obstacles:**

   ```
   Layer name: "cutb_fences"
   Paint tiles where bottom edge should block movement

   Layer name: "cutr_walls"
   Paint tiles where right edge should block movement
   ```

3. **Result:**
   - Characters can walk on any painted navmesh tile
   - Characters cannot cross the edges marked in cut layers
   - Perfect for creating one-way passages, fences, or partial walls

## Multiple Layers

You can have multiple layers of each type:

```
navmesh_ground
navmesh_platforms
cutb_layer1
cutb_layer2
cutt_barriers
```

All layers of the same type are processed together, with cuts applied in the order they are loaded.

## Implementation Files

- `NavMesh.h/cpp` - Core navmesh implementation
- `LevelMap.h/cpp` - Layer categorization and loading
- `LevelV1.h/cpp` - Level initialization and cut application
- `AnimatedDataCharacterNavMeshPlayer.h/cpp` - Player movement with navmesh collision

## Notes

- Layer name matching is **case-insensitive**
- Only the **prefix** needs to match (additional text after the prefix is ignored)
- Tiles with **GID = 0** (empty) are ignored in all layer types
- Cut layers only affect navigation, not rendering
- Multiple tiles can mark the same edge from different sides (safe but redundant)

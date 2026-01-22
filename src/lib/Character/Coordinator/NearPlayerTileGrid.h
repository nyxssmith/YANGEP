#pragma once

#include <vector>

// Forward declarations
class LevelV1;
class AnimatedDataCharacterNavMeshAgent;

// Tile status enum
enum class TileStatus
{
    Empty = 0,
    PlannedOccupiedByAgent = 1,
    PlannedAction = 2,
    OccupiedByAgent = 3,
    OccupiedByAction = 4
};

// Represents a single tile in the near-player grid
struct NearPlayerTile
{
    float worldX;                             // World position X
    float worldY;                             // World position Y
    int tileX;                                // Tile position X
    int tileY;                                // Tile position Y
    int nearPlayerTileX;                      // Near-player grid position X (player at 0,0)
    int nearPlayerTileY;                      // Near-player grid position Y (player at 0,0)
    TileStatus status;                        // Current status of the tile
    AnimatedDataCharacterNavMeshAgent *agent; // Optional pointer to agent (nullptr if none)
};

// NearPlayerTileGrid manages an N x N grid of tiles around the player
// The player is always at the center (0, 0) of this grid
class NearPlayerTileGrid
{
public:
    // Constructor with default grid size
    NearPlayerTileGrid(int gridSize = 7);
    ~NearPlayerTileGrid();

    // Initialize the grid with a specific size
    void initialize(int gridSize);

    // Get the grid size (N for an N x N grid)
    int getGridSize() const;

    // Get a tile at a specific near-player grid position
    // Returns nullptr if out of bounds
    const NearPlayerTile *getTile(int nearPlayerX, int nearPlayerY) const;

    // Get a mutable tile at a specific near-player grid position
    // Returns nullptr if out of bounds
    NearPlayerTile *getTileMutable(int nearPlayerX, int nearPlayerY);

    // Get all tiles
    const std::vector<NearPlayerTile> &getTiles() const;

    // Update the grid based on player's current world/tile position
    void updatePlayerPosition(float playerWorldX, float playerWorldY, int playerTileX, int playerTileY);

    // Render the grid tiles with highlighting
    void render(const LevelV1 &level) const;

private:
    int m_gridSize;                      // N for an N x N grid
    std::vector<NearPlayerTile> m_tiles; // Flat array of tiles (size = gridSize * gridSize)

    // Helper to convert near-player grid coords to flat array index
    int getIndex(int nearPlayerX, int nearPlayerY) const;
};

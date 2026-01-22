#include "NearPlayerTileGrid.h"
#include "LevelV1.h"
#include "HighlightTile.h"
#include <stdio.h>
#include <cute.h>

using namespace Cute;

NearPlayerTileGrid::NearPlayerTileGrid(int gridSize)
    : m_gridSize(0)
{
    initialize(gridSize);
}

NearPlayerTileGrid::~NearPlayerTileGrid()
{
}

void NearPlayerTileGrid::initialize(int gridSize)
{
    if (gridSize <= 0)
    {
        printf("NearPlayerTileGrid: Invalid grid size %d, defaulting to 7\n", gridSize);
        gridSize = 7;
    }

    m_gridSize = gridSize;
    m_tiles.clear();
    m_tiles.resize(gridSize * gridSize);

    // Initialize all tiles to default values
    for (int i = 0; i < gridSize * gridSize; ++i)
    {
        m_tiles[i] = NearPlayerTile{0.0f, 0.0f, 0, 0, 0, 0, TileStatus::Empty, nullptr};
    }

    printf("NearPlayerTileGrid: Initialized with size %dx%d (%d tiles)\n",
           gridSize, gridSize, gridSize * gridSize);
}

int NearPlayerTileGrid::getGridSize() const
{
    return m_gridSize;
}

const NearPlayerTile *NearPlayerTileGrid::getTile(int nearPlayerX, int nearPlayerY) const
{
    int index = getIndex(nearPlayerX, nearPlayerY);
    if (index < 0 || index >= static_cast<int>(m_tiles.size()))
    {
        return nullptr;
    }
    return &m_tiles[index];
}

NearPlayerTile *NearPlayerTileGrid::getTileMutable(int nearPlayerX, int nearPlayerY)
{
    int index = getIndex(nearPlayerX, nearPlayerY);
    if (index < 0 || index >= static_cast<int>(m_tiles.size()))
    {
        return nullptr;
    }
    return &m_tiles[index];
}

const std::vector<NearPlayerTile> &NearPlayerTileGrid::getTiles() const
{
    return m_tiles;
}

void NearPlayerTileGrid::updatePlayerPosition(float playerWorldX, float playerWorldY,
                                              int playerTileX, int playerTileY)
{
    // Calculate the offset to center the grid around the player
    int halfSize = m_gridSize / 2;

    // Update each tile in the grid
    for (int ny = -halfSize; ny <= halfSize; ++ny)
    {
        for (int nx = -halfSize; nx <= halfSize; ++nx)
        {
            int index = getIndex(nx, ny);
            if (index < 0 || index >= static_cast<int>(m_tiles.size()))
            {
                continue;
            }

            NearPlayerTile &tile = m_tiles[index];
            tile.nearPlayerTileX = nx;
            tile.nearPlayerTileY = ny;
            tile.tileX = playerTileX + nx;
            tile.tileY = playerTileY + ny;

            // TODO: Calculate world position based on tile size
            // For now, using a placeholder tile size of 64.0f
            const float tileSize = 64.0f;
            tile.worldX = tile.tileX * tileSize;
            tile.worldY = tile.tileY * tileSize;

            // Note: status and agent pointer are preserved during update
        }
    }
}

int NearPlayerTileGrid::getIndex(int nearPlayerX, int nearPlayerY) const
{
    // Convert from centered coordinates (-halfSize to +halfSize) to array indices (0 to gridSize-1)
    int halfSize = m_gridSize / 2;
    int arrayX = nearPlayerX + halfSize;
    int arrayY = nearPlayerY + halfSize;

    // Bounds check
    if (arrayX < 0 || arrayX >= m_gridSize || arrayY < 0 || arrayY >= m_gridSize)
    {
        return -1;
    }

    return arrayY * m_gridSize + arrayX;
}

void NearPlayerTileGrid::render(const LevelV1 &level) const
{
    // Render each tile in the grid with color based on status
    for (const auto &tile : m_tiles)
    {
        CF_Color color;
        float opacity;

        switch (tile.status)
        {
        case TileStatus::PlannedAction:
            // Dark pink for planned action
            color = cf_make_color_rgb(199, 21, 133); // Medium violet red
            opacity = 0.4f;
            break;
        case TileStatus::PlannedOccupiedByAgent:
            // Purple for planned agent position
            color = cf_make_color_rgb(128, 0, 128);
            opacity = 0.4f;
            break;
        case TileStatus::Empty:
        default:
            // Light pink with lower opacity for empty
            color = cf_make_color_rgb(255, 182, 193);
            opacity = 0.2f;
            break;
        }

        highlightTile(level, tile.tileX, tile.tileY, color, 0.9f, opacity);
    }
}

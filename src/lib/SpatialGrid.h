#pragma once

#include <cute.h>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <mutex>

using namespace Cute;

/**
 * SpatialGrid - A grid-based spatial partitioning system for efficient spatial queries
 *
 * Divides the world into fixed-size cells. Each cell contains indices of entities
 * that overlap with that cell. Useful for:
 * - Rendering only nearby entities
 * - Collision detection
 * - Proximity queries
 */
class SpatialGrid
{
public:
    /**
     * Constructor
     * @param cellSize Size of each grid cell in world units (pixels)
     */
    explicit SpatialGrid(float cellSize = 256.0f);

    /**
     * Set the cell size and clear all data
     * @param cellSize New cell size in world units
     */
    void setCellSize(float cellSize);

    /**
     * Get the current cell size
     */
    float getCellSize() const { return m_cellSize; }

    /**
     * Clear all entities from the grid
     */
    void clear();

    /**
     * Insert an entity into the grid
     * @param entityIndex Index of the entity (used as identifier)
     * @param position World position of the entity
     * @param halfSize Half-size of the entity's bounding box (assumes square)
     */
    void insert(size_t entityIndex, v2 position, float halfSize = 32.0f);

    /**
     * Update an entity's position in the grid
     * @param entityIndex Index of the entity
     * @param oldPosition Previous world position
     * @param newPosition New world position
     * @param halfSize Half-size of the entity's bounding box
     */
    void update(size_t entityIndex, v2 oldPosition, v2 newPosition, float halfSize = 32.0f);

    /**
     * Remove an entity from the grid
     * @param entityIndex Index of the entity
     * @param position Current world position of the entity
     * @param halfSize Half-size of the entity's bounding box
     */
    void remove(size_t entityIndex, v2 position, float halfSize = 32.0f);

    /**
     * Query entities within a rectangular area (AABB)
     * @param bounds The AABB to query
     * @return Vector of entity indices that may be in the area
     */
    std::vector<size_t> queryAABB(CF_Aabb bounds) const;

    /**
     * Query entities near a point within a radius
     * @param center Center point of the query
     * @param radius Radius to search within
     * @return Vector of entity indices that may be within radius
     */
    std::vector<size_t> queryRadius(v2 center, float radius) const;

    /**
     * Get the number of occupied cells
     */
    size_t getOccupiedCellCount() const { return m_cells.size(); }

    /**
     * Debug: Get statistics about the grid
     */
    void debugPrint() const;

    /**
     * Debug: Render the grid cells that contain entities
     * @param camera Camera for coordinate transformation
     */
    void debugRender(const class CFNativeCamera &camera) const;

private:
    // Cell key from grid coordinates
    struct CellKey
    {
        int x;
        int y;

        bool operator==(const CellKey &other) const
        {
            return x == other.x && y == other.y;
        }
    };

    // Hash function for CellKey
    struct CellKeyHash
    {
        size_t operator()(const CellKey &key) const
        {
            // Combine x and y into a single hash
            return std::hash<int>()(key.x) ^ (std::hash<int>()(key.y) << 16);
        }
    };

    // Convert world position to cell key
    CellKey positionToCell(v2 position) const;

    // Get all cell keys that an AABB overlaps
    std::vector<CellKey> getCellsForAABB(CF_Aabb bounds) const;

    // Get all cell keys that an entity at position with halfSize overlaps
    std::vector<CellKey> getCellsForEntity(v2 position, float halfSize) const;

    float m_cellSize;

    // Mutex for thread-safe access
    mutable std::mutex m_mutex;

    // Map from cell key to set of entity indices in that cell
    std::unordered_map<CellKey, std::unordered_set<size_t>, CellKeyHash> m_cells;
};

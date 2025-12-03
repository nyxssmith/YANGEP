#include "SpatialGrid.h"
#include "CFNativeCamera.h"
#include <cstdio>
#include <cmath>

SpatialGrid::SpatialGrid(float cellSize)
    : m_cellSize(cellSize)
{
    if (m_cellSize <= 0.0f)
    {
        m_cellSize = 256.0f;
    }
}

void SpatialGrid::setCellSize(float cellSize)
{
    if (cellSize > 0.0f)
    {
        m_cellSize = cellSize;
        clear();
    }
}

void SpatialGrid::clear()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_cells.clear();
}

SpatialGrid::CellKey SpatialGrid::positionToCell(v2 position) const
{
    return CellKey{
        static_cast<int>(std::floor(position.x / m_cellSize)),
        static_cast<int>(std::floor(position.y / m_cellSize))};
}

std::vector<SpatialGrid::CellKey> SpatialGrid::getCellsForAABB(CF_Aabb bounds) const
{
    std::vector<CellKey> cells;

    int minX = static_cast<int>(std::floor(bounds.min.x / m_cellSize));
    int maxX = static_cast<int>(std::floor(bounds.max.x / m_cellSize));
    int minY = static_cast<int>(std::floor(bounds.min.y / m_cellSize));
    int maxY = static_cast<int>(std::floor(bounds.max.y / m_cellSize));

    for (int y = minY; y <= maxY; ++y)
    {
        for (int x = minX; x <= maxX; ++x)
        {
            cells.push_back(CellKey{x, y});
        }
    }

    return cells;
}

std::vector<SpatialGrid::CellKey> SpatialGrid::getCellsForEntity(v2 position, float halfSize) const
{
    CF_Aabb bounds = cf_make_aabb(
        cf_v2(position.x - halfSize, position.y - halfSize),
        cf_v2(position.x + halfSize, position.y + halfSize));
    return getCellsForAABB(bounds);
}

void SpatialGrid::insert(size_t entityIndex, v2 position, float halfSize)
{
    auto cells = getCellsForEntity(position, halfSize);
    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto &cell : cells)
    {
        m_cells[cell].insert(entityIndex);
    }
}

void SpatialGrid::update(size_t entityIndex, v2 oldPosition, v2 newPosition, float halfSize)
{
    // Get old and new cells
    auto oldCells = getCellsForEntity(oldPosition, halfSize);
    auto newCells = getCellsForEntity(newPosition, halfSize);

    std::lock_guard<std::mutex> lock(m_mutex);

    // Remove from old cells that are not in new cells
    for (const auto &oldCell : oldCells)
    {
        bool stillIn = false;
        for (const auto &newCell : newCells)
        {
            if (oldCell == newCell)
            {
                stillIn = true;
                break;
            }
        }
        if (!stillIn)
        {
            auto it = m_cells.find(oldCell);
            if (it != m_cells.end())
            {
                it->second.erase(entityIndex);
                if (it->second.empty())
                {
                    m_cells.erase(it);
                }
            }
        }
    }

    // Add to new cells that were not in old cells
    for (const auto &newCell : newCells)
    {
        bool wasIn = false;
        for (const auto &oldCell : oldCells)
        {
            if (oldCell == newCell)
            {
                wasIn = true;
                break;
            }
        }
        if (!wasIn)
        {
            m_cells[newCell].insert(entityIndex);
        }
    }
}

void SpatialGrid::remove(size_t entityIndex, v2 position, float halfSize)
{
    auto cells = getCellsForEntity(position, halfSize);
    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto &cell : cells)
    {
        auto it = m_cells.find(cell);
        if (it != m_cells.end())
        {
            it->second.erase(entityIndex);
            if (it->second.empty())
            {
                m_cells.erase(it);
            }
        }
    }
}

std::vector<size_t> SpatialGrid::queryAABB(CF_Aabb bounds) const
{
    std::unordered_set<size_t> resultSet;

    auto cells = getCellsForAABB(bounds);

    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto &cell : cells)
    {
        auto it = m_cells.find(cell);
        if (it != m_cells.end())
        {
            for (size_t idx : it->second)
            {
                resultSet.insert(idx);
            }
        }
    }

    return std::vector<size_t>(resultSet.begin(), resultSet.end());
}

std::vector<size_t> SpatialGrid::queryRadius(v2 center, float radius) const
{
    // Query using AABB first, then caller can do precise distance check
    CF_Aabb bounds = cf_make_aabb(
        cf_v2(center.x - radius, center.y - radius),
        cf_v2(center.x + radius, center.y + radius));
    return queryAABB(bounds);
}

void SpatialGrid::debugPrint() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    printf("=== SpatialGrid Debug ===\n");
    printf("  Cell Size: %.1f\n", m_cellSize);
    printf("  Occupied Cells: %zu\n", m_cells.size());

    size_t totalEntities = 0;
    size_t maxInCell = 0;
    for (const auto &pair : m_cells)
    {
        totalEntities += pair.second.size();
        if (pair.second.size() > maxInCell)
        {
            maxInCell = pair.second.size();
        }
    }

    printf("  Total Entity References: %zu\n", totalEntities);
    printf("  Max Entities in Single Cell: %zu\n", maxInCell);
    if (!m_cells.empty())
    {
        printf("  Average Entities per Cell: %.2f\n",
               static_cast<float>(totalEntities) / m_cells.size());
    }
    printf("========================\n");
}

void SpatialGrid::debugRender(const CFNativeCamera &camera) const
{
    // Get view bounds to only render visible cells
    CF_Aabb viewBounds = camera.getViewBounds();

    // Expand slightly to catch cells on edge
    viewBounds.min.x -= m_cellSize;
    viewBounds.min.y -= m_cellSize;
    viewBounds.max.x += m_cellSize;
    viewBounds.max.y += m_cellSize;

    // Draw all occupied cells that are in view
    cf_draw_push_color(cf_make_color_rgba(100, 100, 255, 100)); // Semi-transparent blue

    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto &pair : m_cells)
    {
        const CellKey &cell = pair.first;
        size_t entityCount = pair.second.size();

        // Calculate cell bounds in world coordinates
        float cellMinX = cell.x * m_cellSize;
        float cellMinY = cell.y * m_cellSize;
        float cellMaxX = cellMinX + m_cellSize;
        float cellMaxY = cellMinY + m_cellSize;

        // Check if cell is in view
        if (cellMaxX < viewBounds.min.x || cellMinX > viewBounds.max.x ||
            cellMaxY < viewBounds.min.y || cellMinY > viewBounds.max.y)
        {
            continue;
        }

        CF_Aabb cellBounds = cf_make_aabb(
            cf_v2(cellMinX, cellMinY),
            cf_v2(cellMaxX, cellMaxY));

        // Color intensity based on entity count (more entities = brighter)
        float intensity = std::min(1.0f, 0.2f + (entityCount * 0.15f));
        cf_draw_push_color(cf_make_color_rgba(
            static_cast<uint8_t>(100 * intensity),
            static_cast<uint8_t>(100 * intensity),
            static_cast<uint8_t>(255 * intensity),
            static_cast<uint8_t>(80 + entityCount * 20)));

        // Draw filled cell
        cf_draw_quad_fill(cellBounds, 0.0f);

        cf_draw_pop_color();

        // Draw cell border
        cf_draw_push_color(cf_make_color_rgba(150, 150, 255, 200));
        cf_draw_quad(cellBounds, 0.0f, 1.0f);
        cf_draw_pop_color();
    }

    cf_draw_pop_color();
}

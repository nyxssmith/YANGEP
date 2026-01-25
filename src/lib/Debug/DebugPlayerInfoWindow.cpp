#include "DebugPlayerInfoWindow.h"
#include "AnimatedDataCharacterNavMeshPlayer.h"
#include "LevelV1.h"
#include <cute.h>
#include <imgui.h>

DebugPlayerInfoWindow::DebugPlayerInfoWindow(const std::string &title,
                                             const AnimatedDataCharacterNavMeshPlayer &player,
                                             const LevelV1 &level)
    : DebugWindow(title), m_player(player), m_level(level)
{
}

void DebugPlayerInfoWindow::render()
{
    if (m_show)
    {
        ImGui::Begin(m_title.c_str(), &m_show);

        // Get player world position
        CF_V2 worldPos = m_player.getPosition();

        // Get tile dimensions from level
        int tileWidth = m_level.getTileWidth();
        int tileHeight = m_level.getTileHeight();

        // Calculate tile coordinates from world position
        float tileX = worldPos.x / static_cast<float>(tileWidth);
        float tileY = worldPos.y / static_cast<float>(tileHeight);

        // Display world coordinates
        ImGui::Text("World Position:");
        ImGui::Indent();
        ImGui::Text("X: %.2f", worldPos.x);
        ImGui::Text("Y: %.2f", worldPos.y);
        ImGui::Unindent();

        ImGui::Separator();

        // Display tile coordinates
        ImGui::Text("Tile Position:");
        ImGui::Indent();
        ImGui::Text("X: %.2f (tile %d)", tileX, static_cast<int>(tileX));
        ImGui::Text("Y: %.2f (tile %d)", tileY, static_cast<int>(tileY));
        ImGui::Unindent();

        ImGui::Separator();

        // Display additional navmesh info if available
        if (m_player.hasNavMesh())
        {
            int currentPoly = m_player.getCurrentPolygon();
            ImGui::Text("NavMesh Info:");
            ImGui::Indent();
            ImGui::Text("On Walkable: %s", m_player.isOnWalkableArea() ? "Yes" : "No");
            ImGui::Text("Current Polygon: %d", currentPoly);
            ImGui::Unindent();
        }
        else
        {
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "No NavMesh assigned");
        }

        ImGui::End();
    }
}

#include "DebugCoordinatorWindow.h"
#include "Coordinator.h"
#include "AnimatedDataCharacter.h"
#include "AnimatedDataCharacterNavMeshAgent.h"
#include "LevelV1.h"
#include <cute.h>
#include <imgui.h>

DebugCoordinatorWindow::DebugCoordinatorWindow(const std::string &title,
                                               Coordinator *coordinator,
                                               const AnimatedDataCharacter *player,
                                               const LevelV1 &level)
    : DebugWindow(title), m_coordinator(coordinator), m_player(player), m_level(level)
{
}

void DebugCoordinatorWindow::render()
{
    if (!m_show || !m_coordinator)
    {
        return;
    }

    ImGui::Begin(m_title.c_str(), &m_show);

    // Get tile dimensions from level
    int tileWidth = m_level.getTileWidth();
    int tileHeight = m_level.getTileHeight();

    // Display player tile position
    if (m_player)
    {
        CF_V2 playerPos = m_player->getPosition();
        float playerTileX = playerPos.x / static_cast<float>(tileWidth);
        float playerTileY = playerPos.y / static_cast<float>(tileHeight);

        ImGui::Text("Player Tile Position:");
        ImGui::Indent();
        ImGui::Text("X: %.2f (tile %d)", playerTileX, static_cast<int>(playerTileX));
        ImGui::Text("Y: %.2f (tile %d)", playerTileY, static_cast<int>(playerTileY));
        ImGui::Unindent();
        ImGui::Separator();
    }

    // Display timing information
    double updateTimeMs = m_coordinator->getLastUpdateTimeMs();
    ImGui::Text("Update Time: %.3f ms", updateTimeMs);
    ImGui::Separator();

    // Display total agent count
    size_t agentCount = m_coordinator->getAgentCount();
    ImGui::Text("Coordinated Agents: %zu", agentCount);
    ImGui::Separator();

    // Get all agents (returns a copy for thread safety)
    auto agents = m_coordinator->getAgents();

    // Display agent tile positions
    if (agents.empty())
    {
        ImGui::Text("No agents currently being coordinated");
    }
    else
    {
        for (size_t i = 0; i < agents.size(); ++i)
        {
            auto agent = agents[i];
            if (agent)
            {
                // Get agent position and convert to tile coordinates
                CF_V2 pos = agent->getPosition();
                float tileX = pos.x / static_cast<float>(tileWidth);
                float tileY = pos.y / static_cast<float>(tileHeight);

                ImGui::Text("Agent %zu: (%.2f, %.2f) - tile (%d, %d)",
                           i, tileX, tileY,
                           static_cast<int>(tileX), static_cast<int>(tileY));
            }
        }
    }

    ImGui::End();
}

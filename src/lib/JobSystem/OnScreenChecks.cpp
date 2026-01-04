#include "OnScreenChecks.h"
#include "JobSystem.h"
#include "CFNativeCamera.h"
#include "LevelV1.h"
#include "Coordinator.h"
#include "AnimatedDataCharacterNavMeshAgent.h"
#include <stdio.h>
#include <atomic>
#include <cmath>
#include <unordered_set>

namespace OnScreenChecks
{
    // Static pointers to game state (set during initialize)
    static v2 *s_playerPosition = nullptr;
    static CFNativeCamera *s_camera = nullptr;
    static LevelV1 *s_level = nullptr;

    // Coordinator for managing on-screen agents
    static Coordinator s_coordinator;

    // Shutdown signal for the worker loop
    static std::atomic<bool> s_shutdownRequested{false};

    void initialize(v2 *playerPosition, CFNativeCamera *camera, LevelV1 *level)
    {
        s_playerPosition = playerPosition;
        s_camera = camera;
        s_level = level;
        s_shutdownRequested = false;
        printf("OnScreenChecks: Initialized with player=%p, camera=%p, level=%p\n",
               (void *)playerPosition, (void *)camera, (void *)level);
    }

    void start()
    {
        // Submit a job to the "onscreenchecks" worker that runs continuously
        JobSystem::submitJob(
            []()
            {
                // printf("OnScreenChecks: Worker loop started\n");

                while (!s_shutdownRequested.load())
                {
                    // Safety check - ensure all pointers are valid
                    if (!s_playerPosition || !s_camera || !s_level)
                    {
                        continue;
                    }

                    // Get view bounds from camera (expanded slightly for edge cases)
                    CF_Aabb viewBounds = s_camera->getViewBounds();
                    viewBounds.min.x -= 64.0f;
                    viewBounds.min.y -= 64.0f;
                    viewBounds.max.x += 64.0f;
                    viewBounds.max.y += 64.0f;

                    const float agentHalfSize = 32.0f;

                    // Query spatial grid for agents in/near the view area
                    std::vector<size_t> nearbyAgents = s_level->getSpatialGrid().queryAABB(viewBounds);
                    std::unordered_set<size_t> nearbySet(nearbyAgents.begin(), nearbyAgents.end());

                    // Update all agents' visibility status
                    size_t agentCount = s_level->getAgentCount();
                    for (size_t i = 0; i < agentCount; ++i)
                    {
                        if (s_shutdownRequested.load())
                            break;

                        auto *agent = s_level->getAgent(i);
                        if (!agent)
                            continue;

                        // If agent is not in the nearby set, mark as off-screen
                        if (nearbySet.find(i) == nearbySet.end())
                        {
                            bool wasOnScreen = agent->getIsOnScreen();
                            agent->setIsOnScreen(false);
                            if (wasOnScreen)
                            {
                                s_coordinator.removeAgent(agent);
                            }
                            continue;
                        }

                        // Agent is nearby - do precise visibility check
                        v2 agentPos = agent->getPosition();
                        CF_Aabb agentBounds = cf_make_aabb(
                            cf_v2(agentPos.x - agentHalfSize, agentPos.y - agentHalfSize),
                            cf_v2(agentPos.x + agentHalfSize, agentPos.y + agentHalfSize));

                        bool visible = s_camera->isVisible(agentBounds);
                        bool wasOnScreen = agent->getIsOnScreen();
                        agent->setIsOnScreen(visible);

                        // Update coordinator based on visibility change
                        if (visible && !wasOnScreen)
                        {
                            s_coordinator.addAgent(agent);
                        }
                        else if (!visible && wasOnScreen)
                        {
                            s_coordinator.removeAgent(agent);
                        }
                    }
                }

                // printf("OnScreenChecks: Worker loop exited\n");
            },
            "OnScreenChecksLoop",
            "onscreenchecks");

        // Kick the job to start it running
        JobSystem::kick();
        // printf("OnScreenChecks: Worker job submitted and kicked\n");
    }

    void requestShutdown()
    {
        // printf("OnScreenChecks: Shutdown requested\n");
        s_shutdownRequested = true;
    }

    void shutdown()
    {
        s_coordinator.clear();
        s_playerPosition = nullptr;
        s_camera = nullptr;
        s_level = nullptr;
        s_shutdownRequested = false;
        // printf("OnScreenChecks: Shutdown complete\n");
    }

    Coordinator *getCoordinator()
    {
        return &s_coordinator;
    }

} // namespace OnScreenChecks

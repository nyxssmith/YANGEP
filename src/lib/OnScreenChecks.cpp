#include "OnScreenChecks.h"
#include "JobSystem.h"
#include "CFNativeCamera.h"
#include "LevelV1.h"
#include <stdio.h>
#include <atomic>
#include <cmath>

namespace OnScreenChecks
{
    // Static pointers to game state (set during initialize)
    static v2 *s_playerPosition = nullptr;
    static CFNativeCamera *s_camera = nullptr;
    static LevelV1 *s_level = nullptr;

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
                printf("OnScreenChecks: Worker loop started\n");

                while (!s_shutdownRequested.load())
                {
                    // Safety check - ensure all pointers are valid
                    if (!s_playerPosition || !s_camera || !s_level)
                    {
                        continue;
                    }

                    // Get player position (read atomically as a copy)
                    v2 playerPos = *s_playerPosition;

                    // Get viewport width and calculate distance threshold
                    // 1.2 * (viewport_width / 2) = 0.6 * viewport_width
                    v2 viewportSize = s_camera->getViewportSize();
                    float distanceThreshold = 1.2f * (viewportSize.x / 2.0f);

                    // Check all agents
                    size_t agentCount = s_level->getAgentCount();
                    const float agentHalfSize = 32.0f; // Match LevelV1 agent bounds size

                    for (size_t i = 0; i < agentCount; ++i)
                    {
                        // Check shutdown between agents to allow quick exit
                        if (s_shutdownRequested.load())
                        {
                            break;
                        }

                        auto *agent = s_level->getAgent(i);
                        if (!agent)
                        {
                            continue;
                        }

                        // Get agent position
                        v2 agentPos = agent->getPosition();

                        // Calculate distance from player to agent
                        float dx = agentPos.x - playerPos.x;
                        float dy = agentPos.y - playerPos.y;
                        float distance = std::sqrt(dx * dx + dy * dy);

                        // Check if within threshold (near player)
                        bool isNearPlayer = distance <= distanceThreshold;

                        if (isNearPlayer)
                        {
                            // Agent is near player - do full viewport visibility check
                            CF_Aabb agentBounds = cf_make_aabb(
                                cf_v2(agentPos.x - agentHalfSize, agentPos.y - agentHalfSize),
                                cf_v2(agentPos.x + agentHalfSize, agentPos.y + agentHalfSize));

                            bool visible = s_camera->isVisible(agentBounds);
                            agent->setIsOnScreen(visible);
                        }
                        else
                        {
                            // Agent is far from player - not on screen
                            agent->setIsOnScreen(false);
                        }
                    }
                }

                printf("OnScreenChecks: Worker loop exited\n");
            },
            "OnScreenChecksLoop",
            "onscreenchecks");

        // Kick the job to start it running
        JobSystem::kick();
        printf("OnScreenChecks: Worker job submitted and kicked\n");
    }

    void requestShutdown()
    {
        printf("OnScreenChecks: Shutdown requested\n");
        s_shutdownRequested = true;
    }

    void shutdown()
    {
        s_playerPosition = nullptr;
        s_camera = nullptr;
        s_level = nullptr;
        s_shutdownRequested = false;
        printf("OnScreenChecks: Shutdown complete\n");
    }

} // namespace OnScreenChecks

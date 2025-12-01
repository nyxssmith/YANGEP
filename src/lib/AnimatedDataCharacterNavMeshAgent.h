#ifndef ANIMATED_DATA_CHARACTER_NAVMESH_AGENT_H
#define ANIMATED_DATA_CHARACTER_NAVMESH_AGENT_H

#include "AnimatedDataCharacter.h"
#include "NavMesh.h"
#include "NavMeshPath.h"
#include "WanderBehavior.h"
#include "WanderOnceBehavior.h"
#include <memory>
#include <atomic>

using namespace Cute;

// Extended AnimatedDataCharacter that is aware of and tracks which NavMesh it is on
class AnimatedDataCharacterNavMeshAgent : public AnimatedDataCharacter
{
public:
    AnimatedDataCharacterNavMeshAgent();
    ~AnimatedDataCharacterNavMeshAgent();

    // Set the navmesh this agent is operating on
    void setNavMesh(NavMesh *navmesh);

    // Get the navmesh this agent is on
    NavMesh *getNavMesh() const;

    // Check if agent has a navmesh assigned
    bool hasNavMesh() const;

    // Get the current polygon the agent is on (-1 if not on mesh or no mesh assigned)
    int getCurrentPolygon() const;

    // Update the agent's current polygon based on position
    void updateCurrentPolygon();

    // Check if agent is currently on walkable area
    bool isOnWalkableArea() const;

    // Override update to track navmesh position
    void update(float dt, v2 moveVector);

    // Background update - submits AI/pathfinding calculations to job system
    // Returns true if a background job was started, false if one is already running
    bool backgroundUpdate(float dt, bool isOnScreen);

    // Check if background job is complete
    bool isBackgroundUpdateComplete() const;

    // Apply the result of the background update (should only be called if isBackgroundUpdateComplete())
    // Returns the move vector calculated in the background
    v2 getBackgroundMoveVector() const;

    // Get the current navigation path
    std::shared_ptr<NavMeshPath> getCurrentNavMeshPath();
    std::shared_ptr<const NavMeshPath> getCurrentNavMeshPath() const;

    // Set the current navigation path
    void setCurrentNavMeshPath(std::shared_ptr<NavMeshPath> path);

    // Clear the current navigation path
    void clearCurrentNavMeshPath();

    // Get the wander behavior
    WanderBehavior *getWanderBehavior();
    const WanderBehavior *getWanderBehavior() const;

    WanderOnceBehavior *getWanderOnceBehavior();
    const WanderOnceBehavior *getWanderOnceBehavior() const;

    // Background update jobs for different scenarios
    void OnScreenBackgroundUpdateJob(float dt);
    void OffScreenBackgroundUpdateJob(float dt);

private:
    // The navmesh this agent is on (non-owning pointer)
    NavMesh *navmesh;

    // Current polygon the agent is in (-1 if not on mesh)
    int currentPolygon;

    // Current navigation path
    std::shared_ptr<NavMeshPath> currentNavMeshPath;

    // Wander behavior for this agent
    WanderBehavior wanderBehavior;
    WanderOnceBehavior wanderOnceBehavior;
    // Background job state
    std::atomic<bool> backgroundJobRunning;
    std::atomic<bool> backgroundJobComplete;
    v2 backgroundMoveVector;

    // Background AI calculation (runs in worker thread)
    void calculateMoveVector(float dt);
};

#endif // ANIMATED_DATA_CHARACTER_NAVMESH_AGENT_H

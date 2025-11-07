#include "AnimatedDataCharacterNavMeshAgent.h"
#include "JobSystem.h"
#include <cute.h>

using namespace Cute;

// Constructor
AnimatedDataCharacterNavMeshAgent::AnimatedDataCharacterNavMeshAgent()
    : AnimatedDataCharacter(), navmesh(nullptr), currentPolygon(-1),
      backgroundJobRunning(false), backgroundJobComplete(false),
      backgroundMoveVector(cf_v2(0.0f, 0.0f))
{
}

// Destructor
AnimatedDataCharacterNavMeshAgent::~AnimatedDataCharacterNavMeshAgent()
{
    // We don't own the navmesh, so we don't delete it
}

// Set the navmesh this agent is operating on
void AnimatedDataCharacterNavMeshAgent::setNavMesh(NavMesh *navmesh)
{
    this->navmesh = navmesh;
    updateCurrentPolygon();
}

// Get the navmesh this agent is on
NavMesh *AnimatedDataCharacterNavMeshAgent::getNavMesh() const
{
    return navmesh;
}

// Check if agent has a navmesh assigned
bool AnimatedDataCharacterNavMeshAgent::hasNavMesh() const
{
    return navmesh != nullptr;
}

// Get the current polygon the agent is on
int AnimatedDataCharacterNavMeshAgent::getCurrentPolygon() const
{
    return currentPolygon;
}

// Update the agent's current polygon based on position
void AnimatedDataCharacterNavMeshAgent::updateCurrentPolygon()
{
    if (!navmesh)
    {
        currentPolygon = -1;
        return;
    }

    // Get the agent's position from parent class
    v2 agentPosition = getPosition();
    CF_V2 cutePosition = cf_v2(agentPosition.x, agentPosition.y);

    currentPolygon = navmesh->findPolygonAt(cutePosition);
}

// Check if agent is currently on walkable area
bool AnimatedDataCharacterNavMeshAgent::isOnWalkableArea() const
{
    if (!navmesh)
    {
        return false;
    }

    // Get the agent's position from parent class
    v2 agentPosition = getPosition();
    CF_V2 cutePosition = cf_v2(agentPosition.x, agentPosition.y);

    return navmesh->isWalkable(cutePosition);
}

// Override update to track navmesh position
void AnimatedDataCharacterNavMeshAgent::update(float dt, v2 moveVector)
{
    // Call parent update
    AnimatedDataCharacter::update(dt, moveVector);

    // Update our current polygon after movement
    if (navmesh)
    {
        updateCurrentPolygon();
    }
}

// Background update - submits AI/pathfinding calculations to job system
bool AnimatedDataCharacterNavMeshAgent::backgroundUpdate(float dt)
{
    // Check if a job is already running
    if (backgroundJobRunning.load())
    {
        return false; // Job already in progress
    }

    // Mark job as running and not complete
    backgroundJobRunning.store(true);
    backgroundJobComplete.store(false);

    // Submit the calculation job to the job system
    JobSystem::submitJob([this, dt]()
                         {
        this->calculateMoveVector(dt);
        
        // Mark job as complete
        this->backgroundJobComplete.store(true);
        this->backgroundJobRunning.store(false); });

    return true; // Job submitted successfully
}

// Check if background job is complete
bool AnimatedDataCharacterNavMeshAgent::isBackgroundUpdateComplete() const
{
    return backgroundJobComplete.load();
}

// Get the result of the background update
v2 AnimatedDataCharacterNavMeshAgent::getBackgroundMoveVector() const
{
    return backgroundMoveVector;
}

// Background AI calculation (runs in worker thread)
void AnimatedDataCharacterNavMeshAgent::calculateMoveVector(float dt)
{
    // TODO: Implement actual AI/pathfinding logic here
    // For now, just a simple movement pattern as placeholder

    // Pick random numbers between -100 and 100
    float moveX = (rand() % 201) - 100.0f; // Random between -100 and 100
    float moveY = (rand() % 201) - 100.0f; // Random between -100 and 100

    // Simulate expensive AI calculation by waiting 3 seconds
    cf_sleep(3000); // Sleep for 3000 milliseconds (3 seconds)

    // Set the move vector
    backgroundMoveVector = cf_v2(moveX, moveY);

    // In a real implementation, this might:
    // - Calculate pathfinding to a target
    // - Evaluate behavioral AI decisions
    // - Check for obstacles
    // - Update steering behaviors
    // etc.
}

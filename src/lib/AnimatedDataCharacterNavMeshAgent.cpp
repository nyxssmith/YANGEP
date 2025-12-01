#include "AnimatedDataCharacterNavMeshAgent.h"
#include "JobSystem.h"
#include <cute.h>

using namespace Cute;

// Constructor
AnimatedDataCharacterNavMeshAgent::AnimatedDataCharacterNavMeshAgent()
    : AnimatedDataCharacter(), navmesh(nullptr), currentPolygon(-1),
      currentNavMeshPath(nullptr),
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
bool AnimatedDataCharacterNavMeshAgent::backgroundUpdate(float dt, bool isOnScreen)
{
    // Check if a job is already running
    if (backgroundJobRunning.load())
    {
        return false; // Job already in progress
    }

    // Mark job as running and not complete
    backgroundJobRunning.store(true);
    backgroundJobComplete.store(false);

    // Submit the calculation job to the job system with a name
    JobSystem::submitJob([this, dt, isOnScreen]()
                         {
        if (isOnScreen)
        {
            this->OnScreenBackgroundUpdateJob(dt);
        }
        else
        {
            this->OffScreenBackgroundUpdateJob(dt);
        }
        
        // Mark job as complete
        this->backgroundJobComplete.store(true);
        this->backgroundJobRunning.store(false); },
                         "Agent AI Update");

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

// Get the current navigation path
std::shared_ptr<NavMeshPath> AnimatedDataCharacterNavMeshAgent::getCurrentNavMeshPath()
{
    return currentNavMeshPath;
}

// Get the current navigation path (const version)
std::shared_ptr<const NavMeshPath> AnimatedDataCharacterNavMeshAgent::getCurrentNavMeshPath() const
{
    return currentNavMeshPath;
}

// Set the current navigation path
void AnimatedDataCharacterNavMeshAgent::setCurrentNavMeshPath(std::shared_ptr<NavMeshPath> path)
{
    currentNavMeshPath = path;
}

// Clear the current navigation path
void AnimatedDataCharacterNavMeshAgent::clearCurrentNavMeshPath()
{
    currentNavMeshPath = nullptr;
}

// Get the wander behavior
WanderBehavior *AnimatedDataCharacterNavMeshAgent::getWanderBehavior()
{
    return &wanderBehavior;
}

// Get the wander behavior (const version)
const WanderBehavior *AnimatedDataCharacterNavMeshAgent::getWanderBehavior() const
{
    return &wanderBehavior;
}

// Background update job for on-screen agents (more detailed AI)
void AnimatedDataCharacterNavMeshAgent::OnScreenBackgroundUpdateJob(float dt)
{
    // Check if navmesh is available
    if (navmesh == nullptr)
    {
        backgroundMoveVector = cf_v2(0.0f, 0.0f);
        return;
    }

    // Get current position
    v2 agentPosition = getPosition();
    CF_V2 currentPosition = cf_v2(agentPosition.x, agentPosition.y);

    // if has a path
    if (currentNavMeshPath && currentNavMeshPath->isValid())
    {
        // if current position is at current waypoint of the path
        if (currentNavMeshPath->isAtCurrentWaypoint(currentPosition))
        {
            // call getnext for the path
            currentNavMeshPath->getNext();
        }

        // Get the current waypoint (without advancing)
        CF_V2 *nextWaypoint = currentNavMeshPath->getCurrent();

        // if nextwaypoint is null
        if (nextWaypoint == nullptr)
        {
            // if current path is not null, mark it as completed for later cleanup
            if (currentNavMeshPath)
            {
                currentNavMeshPath->markCompleted();
            }
            // get new path and exit
            const int wanderRadius = 500;
            // wander forever
            // currentNavMeshPath = wanderBehavior.GetNewPath(*navmesh, currentPosition, wanderRadius);
            // wander once
            currentNavMeshPath = wanderOnceBehavior.GetNewPath(*navmesh, currentPosition, wanderRadius);

            if (!currentNavMeshPath || !currentNavMeshPath->isValid())
            {
                backgroundMoveVector = cf_v2(0.0f, 0.0f);
            }
            return;
        }

        // set a new backgroundMoveVector toward next waypoint
        float dirX = nextWaypoint->x - currentPosition.x;
        float dirY = nextWaypoint->y - currentPosition.y;

        // Normalize and scale to movement speed
        float length = sqrt(dirX * dirX + dirY * dirY);
        if (length > 0.0f)
        {
            dirX = (dirX / length) * 100.0f; // Scale to speed
            dirY = (dirY / length) * 100.0f;
        }

        backgroundMoveVector = cf_v2(dirX, dirY);
    }
    // else
    else
    {
        // get new path from wander behavior
        const int wanderRadius = 500;
        // wander forever
        // currentNavMeshPath = wanderBehavior.GetNewPath(*navmesh, currentPosition, wanderRadius);
        // wander once
        currentNavMeshPath = wanderOnceBehavior.GetNewPath(*navmesh, currentPosition, wanderRadius);

        backgroundMoveVector = cf_v2(0.0f, 0.0f);
    }
}

// Background update job for off-screen agents (simplified AI)
void AnimatedDataCharacterNavMeshAgent::OffScreenBackgroundUpdateJob(float dt)
{
    OnScreenBackgroundUpdateJob(dt);
    // TODO: Implement simplified AI for off-screen agents
    // This might include:
    // - Simpler pathfinding or path following
    // - Basic behavior updates
    // - Minimal state changes
    // - Skip expensive calculations that won't be visible
}

// Background AI calculation (runs in worker thread)
void AnimatedDataCharacterNavMeshAgent::calculateMoveVector(float dt)
{
}

#include "AnimatedDataCharacterNavMeshAgent.h"
#include <cute.h>

using namespace Cute;

// Constructor
AnimatedDataCharacterNavMeshAgent::AnimatedDataCharacterNavMeshAgent()
    : AnimatedDataCharacter(), navmesh(nullptr), currentPolygon(-1)
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
void AnimatedDataCharacterNavMeshAgent::update(float dt)
{
    // Call parent update
    AnimatedDataCharacter::update(dt);

    // Update our current polygon after movement
    if (navmesh)
    {
        updateCurrentPolygon();
    }
}

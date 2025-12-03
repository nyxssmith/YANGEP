#include "AnimatedDataCharacterNavMeshPlayer.h"
#include <cute.h>

using namespace Cute;

// Constructor
AnimatedDataCharacterNavMeshPlayer::AnimatedDataCharacterNavMeshPlayer()
    : AnimatedDataCharacter(), navmesh(nullptr), currentPolygon(-1)
{
}

// Destructor
AnimatedDataCharacterNavMeshPlayer::~AnimatedDataCharacterNavMeshPlayer()
{
    // We don't own the navmesh, so we don't delete it
}

// Set the navmesh this player is operating on
void AnimatedDataCharacterNavMeshPlayer::setNavMesh(NavMesh *navmesh)
{
    this->navmesh = navmesh;
    updateCurrentPolygon();
}

// Get the navmesh this player is on
NavMesh *AnimatedDataCharacterNavMeshPlayer::getNavMesh() const
{
    return navmesh;
}

// Check if player has a navmesh assigned
bool AnimatedDataCharacterNavMeshPlayer::hasNavMesh() const
{
    return navmesh != nullptr;
}

// Get the current polygon the player is on
int AnimatedDataCharacterNavMeshPlayer::getCurrentPolygon() const
{
    return currentPolygon;
}

// Update the player's current polygon based on position
void AnimatedDataCharacterNavMeshPlayer::updateCurrentPolygon()
{
    if (!navmesh)
    {
        currentPolygon = -1;
        return;
    }

    // Get the player's position from parent class
    v2 playerPosition = getPosition();
    CF_V2 cutePosition = cf_v2(playerPosition.x, playerPosition.y);

    currentPolygon = navmesh->findPolygonAt(cutePosition);
}

// Check if player is currently on walkable area
bool AnimatedDataCharacterNavMeshPlayer::isOnWalkableArea() const
{
    if (!navmesh)
    {
        return false;
    }

    // Get the player's position from parent class
    v2 playerPosition = getPosition();
    CF_V2 cutePosition = cf_v2(playerPosition.x, playerPosition.y);

    return navmesh->isWalkable(cutePosition);
}

// Override update to track navmesh position
void AnimatedDataCharacterNavMeshPlayer::update(float dt, v2 moveVector)
{
    // Call parent update
    AnimatedDataCharacter::update(dt, moveVector);

    // Update our current polygon after movement
    if (navmesh)
    {
        updateCurrentPolygon();
    }
}

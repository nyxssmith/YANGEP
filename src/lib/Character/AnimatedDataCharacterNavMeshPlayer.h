#ifndef ANIMATED_DATA_CHARACTER_NAVMESH_PLAYER_H
#define ANIMATED_DATA_CHARACTER_NAVMESH_PLAYER_H

#include "AnimatedDataCharacter.h"
#include "NavMesh.h"

using namespace Cute;

/**
 * AnimatedDataCharacterNavMeshPlayer - Player character with NavMesh awareness
 *
 * Unlike AnimatedDataCharacterNavMeshAgent, this class is designed for
 * player-controlled characters. It doesn't have AI/pathfinding logic,
 * but does track the navmesh for walkable area detection.
 */
class AnimatedDataCharacterNavMeshPlayer : public AnimatedDataCharacter
{
public:
    AnimatedDataCharacterNavMeshPlayer();
    ~AnimatedDataCharacterNavMeshPlayer();

    // Set the navmesh this player is operating on
    void setNavMesh(NavMesh *navmesh);

    // Get the navmesh this player is on
    NavMesh *getNavMesh() const;

    // Check if player has a navmesh assigned
    bool hasNavMesh() const;

    // Get the current polygon the player is on (-1 if not on mesh or no mesh assigned)
    int getCurrentPolygon() const;

    // Update the player's current polygon based on position
    void updateCurrentPolygon();

    // Check if player is currently on walkable area
    bool isOnWalkableArea() const;

    // Update to track navmesh position
    void update(float dt, v2 moveVector);

    // Set the sprite dimensions (used to calculate collision box position)
    void setSpriteDimensions(float width, float height);

    // Get the navmesh collision box (thin horizontal box at player's feet)
    CF_Aabb getNavMeshCollisionBox() const;

    // Debug render the navmesh collision box
    void debugRenderNavMeshCollisionBox() const;

private:
    // The navmesh this player is on (non-owning pointer)
    NavMesh *navmesh;

    // Current polygon the player is in (-1 if not on mesh)
    int currentPolygon;

    // Sprite dimensions for collision box positioning (set after init)
    float spriteWidth;
    float spriteHeight;

    // Collision box height (thin horizontal line at feet)
    static constexpr float COLLISION_BOX_HEIGHT = 4.0f;

    // Check if a future position would be on walkable area
    bool wouldBeOnWalkableArea(v2 futurePosition) const;

};

#endif // ANIMATED_DATA_CHARACTER_NAVMESH_PLAYER_H

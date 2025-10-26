#ifndef ANIMATED_DATA_CHARACTER_NAVMESH_AGENT_H
#define ANIMATED_DATA_CHARACTER_NAVMESH_AGENT_H

#include "AnimatedDataCharacter.h"
#include "NavMesh.h"
#include <memory>

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

private:
    // The navmesh this agent is on (non-owning pointer)
    NavMesh *navmesh;

    // Current polygon the agent is in (-1 if not on mesh)
    int currentPolygon;
};

#endif // ANIMATED_DATA_CHARACTER_NAVMESH_AGENT_H

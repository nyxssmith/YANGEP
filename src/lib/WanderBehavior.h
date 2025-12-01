#pragma once

#include "Behavior.h"

// Wander behavior - agent wanders randomly through the navmesh
class WanderBehavior : public Behavior
{
public:
    WanderBehavior();
    ~WanderBehavior();

    // Get a new random wander path
    std::shared_ptr<NavMeshPath> GetNewPath(NavMesh &navmesh, CF_V2 currentPosition, int radius) override;
};

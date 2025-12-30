#ifndef WANDER_NEW_POSITION_STATE_H
#define WANDER_NEW_POSITION_STATE_H

#include "State.h"

class WanderNewPositionState : public State
{
public:
    WanderNewPositionState();
    ~WanderNewPositionState();

    // Override update to do nothing
    void update(float dt) override;

    // Override GetNewPath to find a random wander position within tiles_radius
    std::shared_ptr<NavMeshPath> GetNewPath(NavMesh &navmesh, CF_V2 currentPosition) override;

    // Get the tiles radius
    int getTilesRadius() const;

protected:
    // Override initFromJson to load tiles_radius from inputs
    void initFromJson() override;

private:
    int tiles_radius;      // Radius in tiles to search for wander position
    bool hasGeneratedPath; // Track if we've generated a path
};

#endif // WANDER_NEW_POSITION_STATE_H

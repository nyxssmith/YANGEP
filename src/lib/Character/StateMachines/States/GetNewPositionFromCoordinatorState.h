#ifndef GET_NEW_POSITION_FROM_COORDINATOR_STATE_H
#define GET_NEW_POSITION_FROM_COORDINATOR_STATE_H

#include "State.h"

class GetNewPositionFromCoordinatorState : public State
{
public:
    GetNewPositionFromCoordinatorState();
    ~GetNewPositionFromCoordinatorState();

    // Override update to do nothing
    void update(float dt) override;

    // Override GetNewPath to find a random wander position within tiles_radius
    std::shared_ptr<NavMeshPath> GetNewPath(NavMesh &navmesh, CF_V2 currentPosition) override;

protected:
    // Override initFromJson to load tiles_radius from inputs
    void initFromJson() override;

private:
    bool hasGeneratedPath; // Track if we've generated a path
};

#endif // GET_NEW_POSITION_FROM_COORDINATOR_STATE_H

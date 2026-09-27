#ifndef GO_TO_TILE_POSITION_STATE_H
#define GO_TO_TILE_POSITION_STATE_H

#include "State.h"

class GoToTilePositionState : public State
{
public:
    GoToTilePositionState();
    ~GoToTilePositionState();

    // Override update to do nothing
    void update(float dt) override;

    // Override GetNewPath to find a random wander position within tiles_radius
    std::shared_ptr<NavMeshPath> GetNewPath(NavMesh &navmesh, CF_V2 currentPosition) override;

    // Get the tiles radius
    int getTileX() const;
    int getTileY() const;

protected:
    // Override initFromJson to load tiles_radius from inputs
    void initFromJson() override;

private:
    int tile_x;            // X coordinate of the target tile
    int tile_y;            // Y coordinate of the target tile
    bool hasGeneratedPath; // Track if we've generated a path
};

#endif // GO_TO_TILE_POSITION_STATE_H

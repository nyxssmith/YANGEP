#ifndef GET_ENTITIES_IN_BOX_STATE_H
#define GET_ENTITIES_IN_BOX_STATE_H

#include "State.h"

class GetEntitiesInBoxState : public State
{
public:
    GetEntitiesInBoxState();
    ~GetEntitiesInBoxState();

    // Override update to do nothing
    void update(float dt) override;

protected:
    // Override initFromJson to load tiles_radius from inputs
    void initFromJson() override;

private:
    int top_left_tile_x = 0;
    int top_left_tile_y = 0;
    int bottom_right_tile_x = 0;
    int bottom_right_tile_y = 0;
};

#endif // GET_ENTITIES_IN_BOX_STATE_H

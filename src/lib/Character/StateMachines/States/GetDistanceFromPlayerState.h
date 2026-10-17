#ifndef GET_DISTANCE_FROM_PLAYER_STATE_H
#define GET_DISTANCE_FROM_PLAYER_STATE_H

#include "State.h"

class GetDistanceFromPlayerState : public State
{
public:
    GetDistanceFromPlayerState();
    ~GetDistanceFromPlayerState();

    // Override update to do nothing
    void update(float dt) override;

protected:
    // Override initFromJson to load tiles_radius from inputs
    void initFromJson() override;

private:
};

#endif // GET_DISTANCE_FROM_PLAYER_STATE_H

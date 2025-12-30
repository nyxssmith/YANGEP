#ifndef MOVE_TO_POSITION_STATE_H
#define MOVE_TO_POSITION_STATE_H

#include "State.h"

class MoveToPositionState : public State
{
public:
    MoveToPositionState();
    ~MoveToPositionState();

    // Override update to check if path is complete
    void update(float dt) override;

protected:
    // Override initFromJson if needed for future inputs
    void initFromJson() override;
};

#endif // MOVE_TO_POSITION_STATE_H

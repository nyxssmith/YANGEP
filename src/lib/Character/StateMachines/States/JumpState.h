#ifndef JUMP_STATE_H
#define JUMP_STATE_H

#include "State.h"
#include <string>

class JumpState : public State
{
public:
    JumpState();
    ~JumpState();

    // Override update to print and complete
    void update(float dt) override;

protected:
    // Override initFromJson to load to_print from inputs
    void initFromJson() override;

private:
    int targetStateIndex; // Index of the state to jump to
};

#endif // JUMP_STATE_H

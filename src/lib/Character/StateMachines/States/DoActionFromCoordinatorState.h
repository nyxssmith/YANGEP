#ifndef DO_ACTION_FROM_COORDINATOR_STATE_H
#define DO_ACTION_FROM_COORDINATOR_STATE_H

#include "State.h"
#include <string>

class DoActionFromCoordinatorState : public State
{
public:
    DoActionFromCoordinatorState();
    ~DoActionFromCoordinatorState();

    // Override update to print and complete
    void update(float dt) override;

    // Get the action to perform
    const std::string &getAction() const;

protected:
    // Override initFromJson to load action from inputs
    void initFromJson() override;

private:
    std::string action; // Action to perform
};

#endif // DO_ACTION_FROM_COORDINATOR_STATE_H

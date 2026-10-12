#ifndef END_IF_STATE_H
#define END_IF_STATE_H

#include "State.h"
#include <string>

class EndIfState : public State
{
public:
    EndIfState();
    ~EndIfState();

    // Override update to print and complete
    void update(float dt) override;

protected:
    // Override initFromJson to load to_print from inputs
    void initFromJson() override;

private:
};

#endif // END_IF_STATE_H

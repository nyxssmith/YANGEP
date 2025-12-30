#ifndef WAIT_STATE_H
#define WAIT_STATE_H

#include "State.h"

class WaitState : public State
{
public:
    WaitState();
    ~WaitState();

    // Override update to handle waiting
    void update(float dt) override;

    // Get wait time
    float getWaitTimeMs() const;

    // Get elapsed time
    float getElapsedTimeMs() const;

    // Check if wait is complete
    bool isComplete() const;

    // Reset the wait timer
    void reset() override;

protected:
    // Override initFromJson to load wait_ms from inputs
    void initFromJson() override;

private:
    float wait_ms;       // Total time to wait in milliseconds
    float elapsedTimeMs; // Time elapsed so far in milliseconds
};

#endif // WAIT_STATE_H

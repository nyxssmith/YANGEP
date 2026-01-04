#include "WaitState.h"
#include <cstdio>

WaitState::WaitState()
    : State(), wait_ms(0.0f), elapsedTimeMs(0.0f)
{
}

WaitState::~WaitState()
{
}

void WaitState::initFromJson()
{
    // Call parent implementation first
    State::initFromJson();

    // Try to read 'ms' from default values
    const DataFile &defaults = getDefaultValues();
    if (defaults.contains("ms"))
    {
        if (defaults["ms"].is_number())
        {
            wait_ms = defaults["ms"].get<float>();
            // printf("WaitState: Loaded wait time of %.2f ms\n", wait_ms);
        }
    }
}

void WaitState::update(float dt)
{
    if (!getIsRunning())
    {
        return;
    }

    // Convert dt from seconds to milliseconds
    float dtMs = dt * 1000.0f;

    elapsedTimeMs += dtMs;

    // Check if wait is complete
    if (elapsedTimeMs >= wait_ms)
    {
        // printf("WaitState: Wait of %.2f ms complete (elapsed: %.2f ms)\n", wait_ms, elapsedTimeMs);
        setIsRunning(false);
    }
}

float WaitState::getWaitTimeMs() const
{
    return wait_ms;
}

float WaitState::getElapsedTimeMs() const
{
    return elapsedTimeMs;
}

bool WaitState::isComplete() const
{
    return elapsedTimeMs >= wait_ms;
}

void WaitState::reset()
{
    // Reset the elapsed time counter
    elapsedTimeMs = 0.0f;
    // printf("WaitState: Reset - waiting for %.2f ms\n", wait_ms);
}

#pragma once

#include <cute.h>

using namespace Cute;

// Forward declarations
class CFNativeCamera;
class LevelV1;
class Coordinator;
class AnimatedDataCharacter;

// On-screen checks job functions
// These functions are designed to run on the "onscreenchecks" dedicated worker thread
// The worker runs a continuous loop checking agent visibility

namespace OnScreenChecks
{
    // Initialize with pointers to player position, camera, and level
    // These pointers must remain valid for the lifetime of the OnScreenChecks system
    void initialize(v2 *playerPosition, CFNativeCamera *camera, LevelV1 *level, const AnimatedDataCharacter *player);

    // Start the on-screen checker worker loop
    // This submits a job that runs continuously until requestShutdown() is called
    void start();

    // Request the worker loop to shutdown
    // The worker will exit on its next iteration
    void requestShutdown();

    // Shutdown and cleanup (call after requestShutdown and job completion)
    void shutdown();

    // Get the coordinator instance for accessing on-screen agents
    Coordinator *getCoordinator();

} // namespace OnScreenChecks

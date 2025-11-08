# Job System Usage Guide

## Overview

The JobSystem is a C++ wrapper around Cute Framework's native threadpool functionality, making it easy to run background tasks in your game.

## Features

- **Easy C++ Lambda Support**: Submit jobs using modern C++ lambdas
- **Automatic Thread Management**: Automatically uses (CPU cores - 1) threads by default
- **Blocking and Non-blocking**: Choose between waiting for jobs or continuing execution
- **Built on Cute Framework**: Uses CF's battle-tested threadpool implementation

## Basic Usage

### 1. Initialize the Job System

```cpp
#include "lib/JobSystem.h"

// In your initialization code (e.g., in main())
if (!JobSystem::initialize()) {
    printf("Failed to initialize job system!\n");
    return -1;
}

// Auto-detects CPU cores and creates (cores - 1) worker threads
// Or specify a custom thread count:
// JobSystem::initialize(4);  // Create 4 worker threads
```

### 2. Submit Jobs

```cpp
// Submit jobs using lambdas
JobSystem::submitJob([]() {
    printf("This runs in a background thread!\n");
    // Do expensive work here
});

// Capture variables
int value = 42;
JobSystem::submitJob([value]() {
    printf("Captured value: %d\n", value);
});

// Multiple jobs
for (int i = 0; i < 100; i++) {
    JobSystem::submitJob([i]() {
        // Process item i
        someExpensiveFunction(i);
    });
}
```

### 3. Execute Jobs

```cpp
// Option 1: Kick and wait (blocking - waits for all jobs to finish)
JobSystem::kickAndWait();
printf("All jobs completed!\n");

// Option 2: Kick without waiting (non-blocking - continues immediately)
JobSystem::kick();
printf("Jobs are running in background...\n");
// Do other work here while jobs execute
```

### 4. Shutdown

```cpp
// In your cleanup code
JobSystem::shutdown();
```

## Example Use Cases

### Loading Assets in Background

```cpp
std::vector<Texture*> textures;
std::mutex textureMutex;

// Submit loading jobs
for (const auto& filepath : textureFiles) {
    JobSystem::submitJob([&, filepath]() {
        Texture* tex = loadTexture(filepath);

        std::lock_guard<std::mutex> lock(textureMutex);
        textures.push_back(tex);
    });
}

JobSystem::kickAndWait();
printf("Loaded %zu textures\n", textures.size());
```

### Parallel Processing

```cpp
std::vector<Entity*> entities = getEntities();
const int chunkSize = 100;

// Process entities in parallel chunks
for (size_t i = 0; i < entities.size(); i += chunkSize) {
    JobSystem::submitJob([&entities, i, chunkSize]() {
        size_t end = std::min(i + chunkSize, entities.size());
        for (size_t j = i; j < end; j++) {
            entities[j]->update();
        }
    });
}

JobSystem::kickAndWait();
```

### Collision Detection

```cpp
struct CollisionResult {
    std::vector<Collision> collisions;
    std::mutex mutex;
};

CollisionResult results;

// Check collisions in parallel
JobSystem::submitJob([&]() {
    auto myCollisions = checkCollisions(groupA, groupB);
    std::lock_guard<std::mutex> lock(results.mutex);
    results.collisions.insert(results.collisions.end(),
                             myCollisions.begin(),
                             myCollisions.end());
});

JobSystem::kickAndWait();
processCollisions(results.collisions);
```

## API Reference

### Static Methods

- `bool initialize(int num_threads = 0)` - Initialize with automatic or custom thread count
- `void shutdown()` - Shutdown and cleanup
- `bool isInitialized()` - Check if initialized
- `void submitJob(std::function<void()> work)` - Submit a job
- `void kickAndWait()` - Execute all pending jobs and wait (blocking)
- `void kick()` - Execute all pending jobs without waiting (non-blocking)
- `int getWorkerCount()` - Get number of worker threads
- `CF_Threadpool* getThreadpool()` - Get underlying CF threadpool (advanced usage)

## Thread Safety Notes

⚠️ **Important**: Jobs run in parallel on different threads. Always use proper synchronization when:

- Accessing shared data structures
- Modifying game state
- Using non-thread-safe APIs

Use mutexes, atomics, or other synchronization primitives to protect shared data.

## Performance Tips

1. **Batch Work**: Group small tasks together - thread overhead can exceed small task benefits
2. **Avoid Contention**: Minimize shared data access between jobs
3. **Balance Load**: Try to make jobs roughly equal in workload
4. **Profile**: Use the FPS profiler to measure actual performance gains

## Integration with Cute Framework

The JobSystem wraps Cute Framework's threadpool (`CF_Threadpool`). For advanced usage, you can access it directly:

```cpp
CF_Threadpool* pool = JobSystem::getThreadpool();
cf_threadpool_add_task(pool, myTaskFunction, myData);
cf_threadpool_kick_and_wait(pool);
```

See Cute Framework's [multithreading documentation](https://randygaul.github.io/cute_framework/#/topics/multithreading) for more details.

#pragma once

#include <cute.h>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <mutex>

// Wrapper class for Cute Framework's threadpool to make it easier to use with C++ lambdas
class JobSystem
{
public:
    // Initialize the job system with a specific number of worker threads
    // Pass 0 to automatically use (CPU cores - 1)
    static bool initialize(int num_threads = 0);

    // Shutdown the job system
    static void shutdown();

    // Check if the job system is initialized
    static bool isInitialized();

    // Submit a job using a C++ lambda or function with a name for tracking
    // The job will be executed when the pool is kicked
    static void submitJob(std::function<void()> work, const std::string &jobName = "Unnamed Job");

    // Kick all pending jobs and wait for them to complete (blocking)
    static void kickAndWait();

    // Kick all pending jobs without waiting (non-blocking)
    static void kick();

    // Get the number of worker threads
    static int getWorkerCount();

    // Get the Cute Framework threadpool pointer (for advanced usage)
    static CF_Threadpool *getThreadpool();

    // Get job information for a specific worker (thread-safe)
    struct WorkerInfo
    {
        int workerId;
        bool isRunning;
        std::string currentJobName;
    };
    static std::vector<WorkerInfo> getWorkerInfo();

    // Get total number of pending jobs
    static int getPendingJobCount();

private:
    static CF_Threadpool *s_threadpool;
    static bool s_initialized;
    static int s_workerCount;

    // Job tracking
    static std::vector<std::string> s_workerCurrentJobs;
    static std::vector<bool> s_workerBusy;
    static std::mutex s_trackingMutex;
    static int s_pendingJobs;

    // Internal callback wrapper for CF_Threadpool
    static void jobCallback(void *userData);

    // Structure to hold job data
    struct JobData
    {
        std::function<void()> work;
        std::string name;
    };
};

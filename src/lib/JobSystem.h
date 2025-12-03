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
    // label: Used to categorize jobs (default: "general")
    static void submitJob(std::function<void()> work, const std::string &jobName = "Unnamed Job", const std::string &label = "general");

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
        std::string label;   // Worker label (e.g., "general")
        int pendingJobCount; // Number of jobs queued for this worker
        int runningJobCount; // Number of jobs currently running
    };
    static std::vector<WorkerInfo> getWorkerInfo();

    // Get total number of pending jobs
    static int getPendingJobCount();

private:
    // Structure to hold job data
    struct JobData
    {
        std::function<void()> work;
        std::string name;
        std::string label; // Job label (e.g., "general")
        int workerIndex;   // Which worker this job is assigned to (-1 if not assigned)
    };

    static CF_Threadpool *s_threadpool;
    static bool s_initialized;
    static int s_workerCount;

    // Job tracking
    static std::vector<std::string> s_workerCurrentJobs;
    static std::vector<bool> s_workerBusy;
    static std::vector<std::string> s_workerLabels;            // Label for each worker
    static std::vector<std::vector<JobData *>> s_workerQueues; // Per-worker job queues
    static std::vector<int> s_workerRunningJobs;               // Running jobs per worker
    static std::vector<JobData *> s_pendingJobs;               // Jobs waiting to be distributed
    static std::mutex s_trackingMutex;

    // Internal callback wrapper for CF_Threadpool
    static void jobCallback(void *userData);

    // Distribute pending jobs to worker queues by label
    static void distributeJobs();
};

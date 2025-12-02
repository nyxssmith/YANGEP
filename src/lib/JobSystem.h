#pragma once

#include <cute.h>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include <queue>

// Wrapper class for Cute Framework's threadpool to make it easier to use with C++ lambdas
// Uses round-robin distribution to ensure all jobs are processed fairly
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
    // Jobs are queued and distributed round-robin to workers on kick()
    static void submitJob(std::function<void()> work, const std::string &jobName = "Unnamed Job");

    // Kick all pending jobs and wait for them to complete (blocking)
    static void kickAndWait();

    // Kick all pending jobs without waiting (non-blocking)
    // Distributes queued jobs round-robin across workers before kicking
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
        int queuedJobs; // Number of jobs queued for this worker
    };
    static std::vector<WorkerInfo> getWorkerInfo();

    // Get total number of pending jobs
    static int getPendingJobCount();

    // Get number of jobs waiting to be distributed
    static int getQueuedJobCount();

private:
    static CF_Threadpool *s_threadpool;
    static bool s_initialized;
    static int s_workerCount;

    // Job tracking
    static std::vector<std::string> s_workerCurrentJobs;
    static std::vector<bool> s_workerBusy;
    static std::mutex s_trackingMutex;
    static int s_pendingJobs;

    // Structure to hold job data
    struct JobData
    {
        std::function<void()> work;
        std::string name;
        int workerIndex; // Which worker this job is assigned to
    };

    // Main job queue - jobs are added here then distributed on kick()
    static std::queue<JobData> s_mainJobQueue;
    static std::mutex s_mainQueueMutex;

    // Per-worker job queues
    static std::vector<std::queue<JobData *>> s_workerQueues;
    static std::vector<std::mutex> s_workerQueueMutexes;
    static std::vector<int> s_workerQueueCounts;

    // Internal callback wrapper for CF_Threadpool
    static void jobCallback(void *userData);

    // Distribute jobs from main queue to worker queues round-robin
    static void distributeJobs();

    // Worker function that processes jobs from its queue
    static void workerProcessQueue(int workerIndex);
};

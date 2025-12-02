#include "JobSystem.h"
#include <thread>
#include <stdio.h>
#include <unordered_map>

// Static member initialization
CF_Threadpool *JobSystem::s_threadpool = nullptr;
bool JobSystem::s_initialized = false;
int JobSystem::s_workerCount = 0;
std::vector<std::string> JobSystem::s_workerCurrentJobs;
std::vector<bool> JobSystem::s_workerBusy;
std::mutex JobSystem::s_trackingMutex;
int JobSystem::s_pendingJobs = 0;

// Fair queue static member initialization
std::queue<void *> JobSystem::s_ownerQueue;
std::unordered_map<void *, std::queue<JobSystem::FairJobData>> JobSystem::s_ownerJobs;
std::unordered_set<void *> JobSystem::s_ownersInProgress;
std::mutex JobSystem::s_fairQueueMutex;

bool JobSystem::initialize(int num_threads)
{
    if (s_initialized)
    {
        printf("JobSystem: Already initialized\n");
        return true;
    }

    // Auto-detect thread count if not specified
    // Use (CPU cores - 1) to account for the main thread
    if (num_threads <= 0)
    {
        num_threads = cf_core_count() - 1;
        if (num_threads <= 0)
        {
            num_threads = 1; // At least one worker thread
        }
    }

    s_workerCount = num_threads;
    printf("JobSystem: Initializing with %d worker threads (detected %d CPU cores)\n",
           num_threads, cf_core_count());

    s_threadpool = cf_make_threadpool(num_threads);
    if (s_threadpool == nullptr)
    {
        printf("JobSystem: Failed to create threadpool\n");
        return false;
    }

    // Initialize worker tracking arrays
    s_workerCurrentJobs.resize(num_threads, "Idle");
    s_workerBusy.resize(num_threads, false);
    s_pendingJobs = 0;

    s_initialized = true;
    return true;
}

void JobSystem::shutdown()
{
    if (!s_initialized)
    {
        return;
    }

    printf("JobSystem: Shutting down\n");

    cf_destroy_threadpool(s_threadpool);
    s_threadpool = nullptr;
    s_initialized = false;
    s_workerCount = 0;

    // Clear tracking data
    s_workerCurrentJobs.clear();
    s_workerBusy.clear();
    s_pendingJobs = 0;

    // Clear fair queue data
    {
        std::lock_guard<std::mutex> lock(s_fairQueueMutex);
        while (!s_ownerQueue.empty())
            s_ownerQueue.pop();
        s_ownerJobs.clear();
        s_ownersInProgress.clear();
    }
}

bool JobSystem::isInitialized()
{
    return s_initialized;
}

void JobSystem::jobCallback(void *userData)
{
    JobData *data = static_cast<JobData *>(userData);
    if (data && data->work)
    {
        // Execute the job
        data->work();

        // Decrement pending jobs counter when job completes
        {
            std::lock_guard<std::mutex> lock(s_trackingMutex);
            if (s_pendingJobs > 0)
            {
                s_pendingJobs--;
            }
        }
    }
    delete data;
}

void JobSystem::submitJob(std::function<void()> work, const std::string &jobName)
{
    if (!s_initialized)
    {
        printf("JobSystem: Not initialized, cannot submit job\n");
        return;
    }

    JobData *data = new JobData{work, jobName};

    {
        std::lock_guard<std::mutex> lock(s_trackingMutex);
        s_pendingJobs++;
    }

    cf_threadpool_add_task(s_threadpool, jobCallback, data);
}

void JobSystem::kickAndWait()
{
    if (!s_initialized)
    {
        printf("JobSystem: Not initialized, cannot kick jobs\n");
        return;
    }

    cf_threadpool_kick_and_wait(s_threadpool);

    // After completion, reset pending jobs
    {
        std::lock_guard<std::mutex> lock(s_trackingMutex);
        s_pendingJobs = 0;
    }
}

void JobSystem::kick()
{
    if (!s_initialized)
    {
        printf("JobSystem: Not initialized, cannot kick jobs\n");
        return;
    }

    // Process fair queue jobs first - submit up to worker count jobs
    for (int i = 0; i < s_workerCount; ++i)
    {
        processNextFairJob();
    }

    cf_threadpool_kick(s_threadpool);
}

int JobSystem::getWorkerCount()
{
    return s_workerCount;
}

CF_Threadpool *JobSystem::getThreadpool()
{
    return s_threadpool;
}

std::vector<JobSystem::WorkerInfo> JobSystem::getWorkerInfo()
{
    std::vector<WorkerInfo> info;

    if (!s_initialized)
    {
        return info;
    }

    std::lock_guard<std::mutex> lock(s_trackingMutex);

    // Since CF threadpool doesn't expose per-worker status,
    // we approximate by showing pending jobs distributed conceptually
    for (int i = 0; i < s_workerCount; ++i)
    {
        WorkerInfo worker;
        worker.workerId = i;
        worker.isRunning = (i < s_pendingJobs);
        worker.currentJobName = worker.isRunning ? "Agent AI Update" : "Idle";
        info.push_back(worker);
    }

    return info;
}

int JobSystem::getPendingJobCount()
{
    std::lock_guard<std::mutex> lock(s_trackingMutex);
    return s_pendingJobs;
}

int JobSystem::getFairQueueOwnerCount()
{
    std::lock_guard<std::mutex> lock(s_fairQueueMutex);
    return static_cast<int>(s_ownerQueue.size());
}

void JobSystem::submitFairJob(std::function<void()> work, void *ownerId, const std::string &jobName)
{
    if (!s_initialized)
    {
        printf("JobSystem: Not initialized, cannot submit fair job\n");
        return;
    }

    std::lock_guard<std::mutex> lock(s_fairQueueMutex);

    // Create the job data
    FairJobData jobData{work, jobName, ownerId};

    // Check if this owner already has jobs queued
    bool ownerExists = (s_ownerJobs.find(ownerId) != s_ownerJobs.end() && !s_ownerJobs[ownerId].empty());
    bool ownerInProgress = (s_ownersInProgress.find(ownerId) != s_ownersInProgress.end());

    // Add the job to this owner's queue
    s_ownerJobs[ownerId].push(jobData);

    // If this owner isn't already in the round-robin queue and not currently being processed, add them
    if (!ownerExists && !ownerInProgress)
    {
        s_ownerQueue.push(ownerId);
    }

    {
        std::lock_guard<std::mutex> trackLock(s_trackingMutex);
        s_pendingJobs++;
    }
}

void JobSystem::fairJobCallback(void *userData)
{
    FairJobData *data = static_cast<FairJobData *>(userData);
    if (data && data->work)
    {
        // Execute the job
        data->work();

        // Decrement pending jobs counter
        {
            std::lock_guard<std::mutex> lock(s_trackingMutex);
            if (s_pendingJobs > 0)
            {
                s_pendingJobs--;
            }
        }

        // After job completes, re-queue the owner if they have more jobs
        void *ownerId = data->ownerId;
        {
            std::lock_guard<std::mutex> lock(s_fairQueueMutex);

            // Remove from in-progress set
            s_ownersInProgress.erase(ownerId);

            // If owner has more jobs, add them back to the end of the queue
            if (s_ownerJobs.find(ownerId) != s_ownerJobs.end() && !s_ownerJobs[ownerId].empty())
            {
                s_ownerQueue.push(ownerId);
            }
        }
    }
    delete data;
}

void JobSystem::processNextFairJob()
{
    std::lock_guard<std::mutex> lock(s_fairQueueMutex);

    if (s_ownerQueue.empty())
    {
        return;
    }

    // Get the next owner in round-robin order
    void *ownerId = s_ownerQueue.front();
    s_ownerQueue.pop();

    // Get their next job
    if (s_ownerJobs.find(ownerId) == s_ownerJobs.end() || s_ownerJobs[ownerId].empty())
    {
        return; // No jobs for this owner (shouldn't happen)
    }

    FairJobData jobData = s_ownerJobs[ownerId].front();
    s_ownerJobs[ownerId].pop();

    // Mark this owner as in-progress so they don't get re-queued until their job completes
    s_ownersInProgress.insert(ownerId);

    // Submit the job to the threadpool
    FairJobData *data = new FairJobData{jobData.work, jobData.name, jobData.ownerId};
    cf_threadpool_add_task(s_threadpool, fairJobCallback, data);
}

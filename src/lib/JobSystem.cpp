#include "JobSystem.h"
#include <thread>
#include <stdio.h>

// Static member initialization
CF_Threadpool *JobSystem::s_threadpool = nullptr;
bool JobSystem::s_initialized = false;
int JobSystem::s_workerCount = 0;
std::vector<std::string> JobSystem::s_workerCurrentJobs;
std::vector<bool> JobSystem::s_workerBusy;
std::mutex JobSystem::s_trackingMutex;
int JobSystem::s_pendingJobs = 0;

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

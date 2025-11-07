#include "JobSystem.h"
#include <thread>
#include <stdio.h>

// Static member initialization
CF_Threadpool *JobSystem::s_threadpool = nullptr;
bool JobSystem::s_initialized = false;
int JobSystem::s_workerCount = 0;

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
        data->work();
    }
    delete data;
}

void JobSystem::submitJob(std::function<void()> work)
{
    if (!s_initialized)
    {
        printf("JobSystem: Not initialized, cannot submit job\n");
        return;
    }

    JobData *data = new JobData{work};
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

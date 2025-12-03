#include "JobSystem.h"
#include <thread>
#include <stdio.h>

// Static member initialization
CF_Threadpool *JobSystem::s_threadpool = nullptr;
bool JobSystem::s_initialized = false;
int JobSystem::s_workerCount = 0;
std::vector<std::string> JobSystem::s_workerCurrentJobs;
std::vector<bool> JobSystem::s_workerBusy;
std::vector<std::string> JobSystem::s_workerLabels;
std::vector<std::vector<JobSystem::JobData *>> JobSystem::s_workerQueues;
std::vector<JobSystem::JobData *> JobSystem::s_pendingJobs;
std::mutex JobSystem::s_trackingMutex;

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
    s_workerLabels.resize(num_threads, "general");
    s_workerQueues.resize(num_threads);
    s_pendingJobs.clear();

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
    s_workerLabels.clear();
    s_workerQueues.clear();

    // Clean up any pending jobs
    for (auto *job : s_pendingJobs)
    {
        delete job;
    }
    s_pendingJobs.clear();
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
    }
    delete data;
}

void JobSystem::submitJob(std::function<void()> work, const std::string &jobName, const std::string &label)
{
    if (!s_initialized)
    {
        printf("JobSystem: Not initialized, cannot submit job\n");
        return;
    }

    JobData *data = new JobData{work, jobName, label};

    {
        std::lock_guard<std::mutex> lock(s_trackingMutex);
        s_pendingJobs.push_back(data);
    }
}

void JobSystem::distributeJobs()
{
    // Must be called with s_trackingMutex held
    // Distribute pending jobs to workers based on label matching

    for (auto *job : s_pendingJobs)
    {
        // Find workers with matching label
        std::vector<int> matchingWorkers;
        for (int i = 0; i < s_workerCount; ++i)
        {
            if (s_workerLabels[i] == job->label)
            {
                matchingWorkers.push_back(i);
            }
        }

        if (matchingWorkers.empty())
        {
            // No matching workers, distribute to worker with smallest queue
            int minQueueWorker = 0;
            size_t minQueueSize = s_workerQueues[0].size();
            for (int i = 1; i < s_workerCount; ++i)
            {
                if (s_workerQueues[i].size() < minQueueSize)
                {
                    minQueueSize = s_workerQueues[i].size();
                    minQueueWorker = i;
                }
            }
            s_workerQueues[minQueueWorker].push_back(job);
        }
        else
        {
            // Distribute to matching worker with smallest queue
            int minQueueWorker = matchingWorkers[0];
            size_t minQueueSize = s_workerQueues[matchingWorkers[0]].size();
            for (size_t i = 1; i < matchingWorkers.size(); ++i)
            {
                int workerId = matchingWorkers[i];
                if (s_workerQueues[workerId].size() < minQueueSize)
                {
                    minQueueSize = s_workerQueues[workerId].size();
                    minQueueWorker = workerId;
                }
            }
            s_workerQueues[minQueueWorker].push_back(job);
        }
    }

    s_pendingJobs.clear();
}

void JobSystem::kickAndWait()
{
    if (!s_initialized)
    {
        printf("JobSystem: Not initialized, cannot kick jobs\n");
        return;
    }

    // Distribute pending jobs to worker queues
    {
        std::lock_guard<std::mutex> lock(s_trackingMutex);
        distributeJobs();

        // Submit all queued jobs to the threadpool
        for (int i = 0; i < s_workerCount; ++i)
        {
            for (auto *job : s_workerQueues[i])
            {
                cf_threadpool_add_task(s_threadpool, jobCallback, job);
            }
            s_workerQueues[i].clear();
        }
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

    // Distribute pending jobs to worker queues
    {
        std::lock_guard<std::mutex> lock(s_trackingMutex);
        distributeJobs();

        // Submit all queued jobs to the threadpool
        for (int i = 0; i < s_workerCount; ++i)
        {
            for (auto *job : s_workerQueues[i])
            {
                cf_threadpool_add_task(s_threadpool, jobCallback, job);
            }
            s_workerQueues[i].clear();
        }
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

    for (int i = 0; i < s_workerCount; ++i)
    {
        WorkerInfo worker;
        worker.workerId = i;
        worker.isRunning = s_workerBusy[i];
        worker.currentJobName = s_workerCurrentJobs[i];
        worker.label = s_workerLabels[i];
        worker.pendingJobCount = static_cast<int>(s_workerQueues[i].size());
        info.push_back(worker);
    }

    return info;
}

int JobSystem::getPendingJobCount()
{
    std::lock_guard<std::mutex> lock(s_trackingMutex);
    int total = static_cast<int>(s_pendingJobs.size());
    for (int i = 0; i < s_workerCount; ++i)
    {
        total += static_cast<int>(s_workerQueues[i].size());
    }
    return total;
}

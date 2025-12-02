#include "JobSystem.h"
#include "DebugPrint.h"
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

// Main queue and per-worker queues
std::queue<JobSystem::JobData> JobSystem::s_mainJobQueue;
std::mutex JobSystem::s_mainQueueMutex;
std::vector<std::queue<JobSystem::JobData *>> JobSystem::s_workerQueues;
std::vector<std::mutex> JobSystem::s_workerQueueMutexes;
std::vector<int> JobSystem::s_workerQueueCounts;

bool JobSystem::initialize(int num_threads)
{
    if (s_initialized)
    {
        DebugPrint::Print("JobSystem", "JobSystem: Already initialized\n");
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
    // TODO tune this either 1 per core pinned, or N processes
    //  double the number of worker threads
    num_threads *= 2;
    s_workerCount = num_threads;
    DebugPrint::Print("JobSystem", "JobSystem: Initializing with %d worker threads (detected %d CPU cores)\n",
                      num_threads, cf_core_count());

    s_threadpool = cf_make_threadpool(num_threads);
    if (s_threadpool == nullptr)
    {
        DebugPrint::Print("JobSystem", "JobSystem: Failed to create threadpool\n");
        return false;
    }

    // Initialize worker tracking arrays
    s_workerCurrentJobs.resize(num_threads, "Idle");
    s_workerBusy.resize(num_threads, false);
    s_pendingJobs = 0;

    // Initialize per-worker queues
    s_workerQueues.resize(num_threads);
    s_workerQueueMutexes = std::vector<std::mutex>(num_threads);
    s_workerQueueCounts.resize(num_threads, 0);

    s_initialized = true;
    return true;
}

void JobSystem::shutdown()
{
    if (!s_initialized)
    {
        return;
    }

    DebugPrint::Print("JobSystem", "JobSystem: Shutting down\n");

    cf_destroy_threadpool(s_threadpool);
    s_threadpool = nullptr;
    s_initialized = false;
    s_workerCount = 0;

    // Clear tracking data
    s_workerCurrentJobs.clear();
    s_workerBusy.clear();
    s_pendingJobs = 0;

    // Clear main queue
    {
        std::lock_guard<std::mutex> lock(s_mainQueueMutex);
        while (!s_mainJobQueue.empty())
            s_mainJobQueue.pop();
    }

    // Clear worker queues
    for (int i = 0; i < static_cast<int>(s_workerQueues.size()); ++i)
    {
        std::lock_guard<std::mutex> lock(s_workerQueueMutexes[i]);
        while (!s_workerQueues[i].empty())
        {
            delete s_workerQueues[i].front();
            s_workerQueues[i].pop();
        }
    }
    s_workerQueues.clear();
    s_workerQueueMutexes.clear();
    s_workerQueueCounts.clear();
}

bool JobSystem::isInitialized()
{
    return s_initialized;
}

void JobSystem::jobCallback(void *userData)
{
    int workerIndex = *static_cast<int *>(userData);
    delete static_cast<int *>(userData);

    // Process all jobs in this worker's queue
    while (true)
    {
        JobData *jobData = nullptr;

        // Get next job from this worker's queue
        {
            std::lock_guard<std::mutex> lock(s_workerQueueMutexes[workerIndex]);
            if (s_workerQueues[workerIndex].empty())
            {
                break; // No more jobs
            }
            jobData = s_workerQueues[workerIndex].front();
            s_workerQueues[workerIndex].pop();
            s_workerQueueCounts[workerIndex]--;
        }

        if (jobData && jobData->work)
        {
            // Update worker status
            {
                std::lock_guard<std::mutex> lock(s_trackingMutex);
                s_workerCurrentJobs[workerIndex] = jobData->name;
                s_workerBusy[workerIndex] = true;
            }

            // Execute the job
            jobData->work();

            // Update tracking
            {
                std::lock_guard<std::mutex> lock(s_trackingMutex);
                if (s_pendingJobs > 0)
                {
                    s_pendingJobs--;
                }
                s_workerCurrentJobs[workerIndex] = "Idle";
                s_workerBusy[workerIndex] = false;
            }
        }

        delete jobData;
    }
}

void JobSystem::submitJob(std::function<void()> work, const std::string &jobName)
{
    if (!s_initialized)
    {
        DebugPrint::Print("JobSystem", "JobSystem: Not initialized, cannot submit job\n");
        return;
    }

    // Add job to main queue - will be distributed on kick()
    {
        std::lock_guard<std::mutex> lock(s_mainQueueMutex);
        s_mainJobQueue.push(JobData{work, jobName, -1});
    }

    {
        std::lock_guard<std::mutex> lock(s_trackingMutex);
        s_pendingJobs++;
    }
}

void JobSystem::distributeJobs()
{
    std::lock_guard<std::mutex> lock(s_mainQueueMutex);

    int workerIndex = 0;

    // Distribute jobs round-robin to worker queues
    while (!s_mainJobQueue.empty())
    {
        JobData job = s_mainJobQueue.front();
        s_mainJobQueue.pop();

        job.workerIndex = workerIndex;

        // Add to worker's queue
        {
            std::lock_guard<std::mutex> workerLock(s_workerQueueMutexes[workerIndex]);
            s_workerQueues[workerIndex].push(new JobData{job.work, job.name, workerIndex});
            s_workerQueueCounts[workerIndex]++;
        }

        // Round-robin to next worker
        workerIndex = (workerIndex + 1) % s_workerCount;
    }
}

void JobSystem::kickAndWait()
{
    if (!s_initialized)
    {
        DebugPrint::Print("JobSystem", "JobSystem: Not initialized, cannot kick jobs\n");
        return;
    }

    // Distribute jobs to worker queues
    distributeJobs();

    // Submit one task per worker to process their queue
    for (int i = 0; i < s_workerCount; ++i)
    {
        int *workerIndexPtr = new int(i);
        cf_threadpool_add_task(s_threadpool, jobCallback, workerIndexPtr);
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
        DebugPrint::Print("JobSystem", "JobSystem: Not initialized, cannot kick jobs\n");
        return;
    }

    // Distribute jobs to worker queues
    distributeJobs();

    // Submit one task per worker to process their queue
    for (int i = 0; i < s_workerCount; ++i)
    {
        int *workerIndexPtr = new int(i);
        cf_threadpool_add_task(s_threadpool, jobCallback, workerIndexPtr);
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
        worker.queuedJobs = s_workerQueueCounts[i];
        info.push_back(worker);
    }

    return info;
}

int JobSystem::getPendingJobCount()
{
    std::lock_guard<std::mutex> lock(s_trackingMutex);
    return s_pendingJobs;
}

int JobSystem::getQueuedJobCount()
{
    std::lock_guard<std::mutex> lock(s_mainQueueMutex);
    return static_cast<int>(s_mainJobQueue.size());
}

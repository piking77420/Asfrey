#pragma once
#include <array>
#include <mutex>
#include <queue>
#include <thread>

#include "Asfrey_header.h"
#include "Fiber.hpp"


namespace Asfrey
{
    struct Job;

    struct JobCounter
    {
        std::atomic_uint currentJobBeExecute;
        Job* jobPtr = nullptr;
        size_t JobNbr = 0;
    };
   
    class AsfreyPool
    {
    public:
        void Initialize();

        void Destroy();

        void RunJob(Job* _job, const size_t _jobNbr);

        void WaitForCounter();
        
        static inline size_t StackSize = -1;
    private:
        struct AsfreyWorkerThread
        {
            std::thread thread;
            std::queue<Job*> m_ThreadPending;
            std::mutex queuMutex;
        };

        std::array<AsfreyWorkerThread, MAX_WORKER_THREAD> m_Threads;
        
        std::array<Fiber, MAX_FIBER> m_Fibers;
        
        JobCounter m_JobCounter;

        void RunThread();
    };
 
}

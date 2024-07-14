#pragma once
#include <array>
#include <mutex>
#include <queue>
#include <thread>

#include "Asfrey_header.h"
#include "Fiber.hpp"
#include "Job.hpp"


namespace Asfrey
{

    class AsfreyPool
    {
    public:
        void Initialize();

        void Destroy();

        void RunJob(Job* _job, const size_t _jobNbr, AtomicCounter** _jobCounter);

        void WaitForCounterAndFree(AtomicCounter* _jobCounter);

        void SetCondition(JobCondition _jobCondition, bool _value);

        bool GetCondition(JobCondition _jobCondition);
        
        static inline size_t StackSize = -1;

    private:
        struct JobQueue
        {
            std::queue<Job*> queue;
            std::mutex queuMutex;
        };

        std::array<std::thread, MAX_WORKER_THREAD> m_Threads;

        std::array<JobQueue, static_cast<size_t>(JobPriorities::COUNT)> m_JobQueue;

        std::array<Fiber, MAX_FIBER> m_Fibers;

        std::unordered_map<JobCondition, bool> m_Condition;

        void RunThread();

        void ThreadRunFunction();

    };
 
}

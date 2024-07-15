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
    // TO DO NEED TO CHANGE THIS to static
    class AsfreyPool
    {
    private:
        static inline AsfreyPool* m_Instance = nullptr;
    public:

        static void __stdcall FibersRunJob(void* param);

        static void Initialize();

        static void Destroy();

        static void RunJob(Job* _job, const size_t _jobNbr, AtomicCounter** _jobCounter);

        static void WaitForCounterAndFree(AtomicCounter* _jobCounter);

        static void SetCondition(JobCondition _jobCondition, bool _value);

        static bool GetCondition(JobCondition _jobCondition);

        static void YieldOnCondition(JobCondition _jobCondition);
        
        static inline size_t StackSize = -1;

    private:
        struct JobQueue
        {
            std::queue<Job*> queue;
            std::mutex queuMutex;
        };

        struct FiberBuffer
        {
           std::array<Fiber, MAX_FIBER> m_Fibers;
           std::mutex fibersBufferMutex;
        };

        struct JobAndWorkerFiber
        {
            Job* job = nullptr;
            Fiber* workerFiber = nullptr;
        };

        struct WorkerThread
        {
            Fiber runthreadFiber;
            std::thread thread;
        };

        std::array<WorkerThread, MAX_WORKER_THREAD> m_Threads;

        std::array<JobQueue, static_cast<size_t>(JobPriorities::COUNT)> m_JobQueue;

        FiberBuffer fiberBuffer;

        std::unordered_map<JobCondition, bool> m_Condition;

        AsfreyPool() = default;

        ~AsfreyPool() = default;

        static void RunThread();


        static void __stdcall ThreadRunFunction(Fiber* _workerFibers);

        static void FinAvailableFiber(Fiber* _outFiber);

    };
 
}

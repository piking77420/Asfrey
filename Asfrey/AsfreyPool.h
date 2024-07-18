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
   

    enum class JobCondition
    {
        C1,
        C2,

        COUNT
    };

   
    enum class FibersWorkingState
    {
        AVAILABLE,
        WAITING,
        WORKING,

        Count
    };

  
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

        struct FiberWithState
        {
            FibersWorkingState state = FibersWorkingState::AVAILABLE;
            Fiber fiber;
        };

        struct FiberBuffer
        {
           std::array<FiberWithState, MAX_FIBER> fibers;
           std::mutex mutex;
        };

        struct FiberPendingMap
        {
            std::unordered_map<JobCondition, FiberBuffer> pendingFiberMap;
        };

        struct JobAndWorkerFiber
        {
            Job* job = nullptr;
            Fiber* workerFiber = nullptr;
        };

        struct WorkerThread
        {
            Fiber runthreadFiber;
            std::jthread thread;
            std::jthread::id id;

            bool hasBeenStopByCondition = false;
        };

   
        std::array<WorkerThread, MAX_WORKER_THREAD> m_jThreads;

        std::array<JobQueue, static_cast<size_t>(JobPriorities::COUNT)> m_JobQueue;

        FiberBuffer m_FiberBuffer;

        FiberPendingMap m_FibersPendingMap;

        std::unordered_map<JobCondition, bool> m_Condition;

        AsfreyPool() = default;

        ~AsfreyPool() = default;

        static void RunThread();

        static void ThreadRunFunction(WorkerThread* _workerThread);

        static bool FindFiber(FiberBuffer* _fiberBuffer, FiberWithState** _outFiber, FibersWorkingState _inState, FibersWorkingState _outState);

        static bool FindFiber(FiberPendingMap* _fiberPendingMap, FiberWithState** _outFiber, FibersWorkingState _inState, FibersWorkingState _outState);

        static bool PutInWaitList(FiberWithState** _fiberToPutInWaitList);

        static bool GetJobFromQueu(JobQueue* _JobQueue,Job* _Outjob);

        static bool LookForWaitingJob();


    };
 
}

#include "AsfreyPool.h"

#include <mutex>
#include <Windows.h>

#include "Job.hpp"

using namespace Asfrey;

void AsfreyPool::Initialize()
{
	ULONG_PTR lowLimit, highLimit;
	GetCurrentThreadStackLimits(&lowLimit, &highLimit);

	// Calculate the stack size
	StackSize = highLimit - lowLimit;
	RunThread();
}

void AsfreyPool::Destroy()
{
	for (size_t i = 0; i < m_Threads.size(); i++)
	{
		AsfreyWorkerThread& asfreyWorker = m_Threads[i];
		asfreyWorker.thread = {};
	}
}

void AsfreyPool::RunJob(Job* _job, const size_t _jobNbr)
{
	m_JobCounter.JobNbr = _jobNbr;
	m_JobCounter.jobPtr = _job;


	for (size_t i = 0; i < _jobNbr; i++)
	{
		const size_t threadIndex = i % MAX_WORKER_THREAD;

		AsfreyWorkerThread& asfreyWorker = m_Threads[threadIndex];
		if (asfreyWorker.queuMutex.try_lock())
		{
			asfreyWorker.m_ThreadPending.push(&_job[i]);
			asfreyWorker.queuMutex.unlock();
		}
		
	}

}

void AsfreyPool::WaitForCounter()
{
	while (m_JobCounter.currentJobBeExecute != m_JobCounter.JobNbr)
	{
		std::this_thread::yield();
	}

	m_JobCounter.currentJobBeExecute = 0;
	m_JobCounter.JobNbr = 0;
	m_JobCounter.jobPtr = nullptr;
}

void AsfreyPool::RunThread()
{
	for (auto& asfreyWorker : m_Threads)
	{
		auto l = [&]()
			{
				while (true)
				{
					if (asfreyWorker.m_ThreadPending.empty())
					{
						continue;
					}

					if (asfreyWorker.queuMutex.try_lock())
					{
						Job* job = asfreyWorker.m_ThreadPending.front();

						job->func(job->arg);
						++m_JobCounter.currentJobBeExecute;
						asfreyWorker.m_ThreadPending.pop();

						asfreyWorker.queuMutex.unlock();
					}
					
				}
			};
		asfreyWorker.thread = std::thread(l);
		asfreyWorker.thread.detach();
	}
}



#include "AsfreyPool.h"

#include <mutex>
#include <Windows.h>

#include "Job.hpp"
#include <iostream>

using namespace Asfrey;

void __stdcall AsfreyPool::FibersRunJob(void* param)
{
	JobAndWorkerFiber* jobAndWorkerFiber = static_cast<JobAndWorkerFiber*>(param);

	(*jobAndWorkerFiber->job)();
	jobAndWorkerFiber->workerFiber->Switch();
}

void AsfreyPool::Initialize()
{
	if (m_Instance != nullptr)
	{
		return;
	}
	m_Instance = new AsfreyPool();

	{
		ULONG_PTR lowLimit, highLimit;
		GetCurrentThreadStackLimits(&lowLimit, &highLimit);

		// Calculate the stack size
		StackSize = highLimit - lowLimit;
	}
	
	{
		for (size_t i = 0; i < static_cast<size_t>(JobCondition::COUNT); i++)
		{
			JobCondition jobCondition = static_cast<JobCondition>(i);
			m_Instance->m_Condition.insert({ jobCondition, false });
		}
	}
	
	m_Instance->RunThread();
}

void AsfreyPool::Destroy()
{
	for (size_t i = 0; i < m_Instance->m_Threads.size(); i++)
	{
		m_Instance->m_Threads[i] = {};
	}

	
}

void AsfreyPool::RunJob(Job* _job, const size_t _jobNbr, AtomicCounter** _jobCounter)
{
	AtomicCounter* atomicCounter = new AtomicCounter();
	*atomicCounter = static_cast<uint32_t>(_jobNbr);
	*_jobCounter = atomicCounter;

	for (size_t i = 0; i < _jobNbr; i++)
	{
		const size_t threadIndex = i % MAX_WORKER_THREAD;

		Job& j = _job[i];
		JobQueue& jobQueu = m_Instance->m_JobQueue[static_cast<size_t>(j.jobPriorities)];
		j.counter = *_jobCounter;

		std::lock_guard<std::mutex> lock(jobQueu.queuMutex);
		jobQueu.queue.push(&j);
	}

}


void AsfreyPool::WaitForCounterAndFree(AtomicCounter* _jobCounter)
{
	while (*_jobCounter != 0)
	{
		std::this_thread::yield();
	}

	delete _jobCounter;
}

void AsfreyPool::SetCondition(JobCondition _jobCondition, bool _value)
{
	m_Instance->m_Condition.at(_jobCondition) = _value;
}

bool AsfreyPool::GetCondition(JobCondition _jobCondition)
{
	return m_Instance->m_Condition.at(_jobCondition);
}

void Asfrey::AsfreyPool::YieldOnCondition(JobCondition _jobCondition)
{
	if (!GetCondition(_jobCondition))
	{

	}

}

void AsfreyPool::RunThread()
{
	for (auto& asfreyWorker : m_Instance->m_Threads)
	{
		auto l = [&]()
			{
				
				asfreyWorker.runthreadFiber.ConvertCurrentThreadToFiber();
				
				while (true)
				{	
					ThreadRunFunction(&asfreyWorker.runthreadFiber);
				}
				asfreyWorker.runthreadFiber.ConvertCurrentFiberToThread();

			};
		asfreyWorker.thread = std::thread(l);
		asfreyWorker.thread.detach();
	}
}

void Asfrey::AsfreyPool::ThreadRunFunction(Fiber* _workerFibers)
{

	for (size_t i = m_Instance->m_JobQueue.size(); i-- > 0;)
	{
		JobQueue& jobQueue = m_Instance->m_JobQueue[i];

		if (jobQueue.queue.empty())
		{
			continue;
		}

		if (jobQueue.queuMutex.try_lock())
		{
			Job job = *jobQueue.queue.front();
			AtomicCounter* atomicCounter = job.counter;

			jobQueue.queue.pop();
			// Free the mutex just before the job execution 
			jobQueue.queuMutex.unlock();

			if (job.func)
			{
				Fiber f;
				FinAvailableFiber(&f);
				
				JobAndWorkerFiber jobAndWorkerFiber =
				{
					.job = &job,
					.workerFiber = _workerFibers
				};
				f.Create(1024, FibersRunJob, &jobAndWorkerFiber);
				f.Switch();
				(*atomicCounter)--;
				f.Destroy();

				// Reset iterattion to always execute The High priorities Job First
				i = m_Instance->m_JobQueue.size();
			}
		}
	}
	
}



void Asfrey::AsfreyPool::FinAvailableFiber(Fiber* _outFiber)
{
	
	if (m_Instance->fiberBuffer.fibersBufferMutex.try_lock())
	{
		for (auto& it : m_Instance->fiberBuffer.m_Fibers )
		{
			if (it)
			{
				_outFiber = &it;
				m_Instance->fiberBuffer.fibersBufferMutex.unlock();
				return;
			}
		}
		m_Instance->fiberBuffer.fibersBufferMutex.unlock();
	}
	
}



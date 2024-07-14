#include "AsfreyPool.h"

#include <mutex>
#include <Windows.h>

#include "Job.hpp"

using namespace Asfrey;

void AsfreyPool::Initialize()
{
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
			m_Condition.insert({ jobCondition, false });
		}
	}
	
	RunThread();
}

void AsfreyPool::Destroy()
{
	for (size_t i = 0; i < m_Threads.size(); i++)
	{
		m_Threads[i] = {};
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
		JobQueue& jobQueu = m_JobQueue[static_cast<size_t>(j.jobPriorities)];
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
	m_Condition.at(_jobCondition) = _value;
}

bool AsfreyPool::GetCondition(JobCondition _jobCondition)
{
	return m_Condition.at(_jobCondition);
}

void AsfreyPool::RunThread()
{
	for (auto& asfreyWorker : m_Threads)
	{
		auto l = [&]()
			{
				while (true)
				{	
					ThreadRunFunction();
				}
			};
		asfreyWorker = std::thread(l);
		asfreyWorker.detach();
	}
}

void Asfrey::AsfreyPool::ThreadRunFunction()
{

	for (size_t i = m_JobQueue.size(); i-- > 0;)
	{
		JobQueue& jobQueue = m_JobQueue[i];

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
				job.func(job.arg);
				(*atomicCounter)--;

				// Reset iterattion to always execute The High priorities Job First
				i = m_JobQueue.size();
			}
		}
	}

}



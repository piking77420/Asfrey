#include "AsfreyPool.h"

#include <mutex>
#include <Windows.h>

#include "Job.hpp"
#include <iostream>

using namespace Asfrey;

void __stdcall AsfreyPool::FibersRunJob(void* _param)
{
	JobAndWorkerFiber* jobAndWorkerFiber = static_cast<JobAndWorkerFiber*>(_param);

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
			//m_Instance->m_FibersPendingMap.pendingFiberMap.at(jobCondition) = std::array<FiberWithState, MAX_FIBER>();
		}
	}



	m_Instance->RunThread();
}

void AsfreyPool::Destroy()
{
	for (size_t i = 0; i < m_Instance->m_jThreads.size(); i++)
	{
		m_Instance->m_jThreads[i] = {};
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
		for (auto& i : m_Instance->m_jThreads)
		{
			std::thread::id current = std::this_thread::get_id();
			std::thread::id lookat = i.thread.get_id();

			if (current != current)
				continue;

			// Return to basic routine 
			i.hasBeenStopByCondition = true;
			i.runthreadFiber.Switch();
		}
	}
}

void AsfreyPool::RunThread()
{
	for (auto& asfreyWorker : m_Instance->m_jThreads)
	{
		auto l = [&]()
			{

				asfreyWorker.runthreadFiber.ConvertCurrentThreadToFiber();

				while (true)
				{
					ThreadRunFunction(&asfreyWorker);
				}
				asfreyWorker.runthreadFiber.ConvertCurrentFiberToThread();

			};
		asfreyWorker.thread = std::jthread(l);
		asfreyWorker.id = asfreyWorker.thread.get_id();
	}
}

void Asfrey::AsfreyPool::ThreadRunFunction(WorkerThread* _workerThread)
{

	for (size_t i = m_Instance->m_JobQueue.size(); i-- > 0;)
	{
		JobQueue& jobQueue = m_Instance->m_JobQueue[i];


		if (jobQueue.queue.empty())
		{
			continue;
		}

		Job job;
		if (!GetJobFromQueu(&jobQueue, &job)) continue;

		if (!job.func || job.counter == nullptr) continue;

		FiberWithState* fiberWithState = nullptr;
		if (!FindFiber(&m_Instance->m_FiberBuffer, &fiberWithState, FibersWorkingState::AVAILABLE, FibersWorkingState::WORKING))
		{
			continue;
		}

		JobAndWorkerFiber jobAndWorkerFiber =
		{
			.job = &job,
			.workerFiber = &_workerThread->runthreadFiber
		};

		fiberWithState->fiber.Create(1024, FibersRunJob, &jobAndWorkerFiber);
		fiberWithState->state = FibersWorkingState::WORKING;
		fiberWithState->fiber.Switch();


		if (_workerThread->hasBeenStopByCondition)
		{
			fiberWithState->state = FibersWorkingState::WAITING;
			// To create a wait List 
			PutInWaitList(&fiberWithState);
			_workerThread->hasBeenStopByCondition = false;
		}
		else
		{
			(*job.counter)--;
			fiberWithState->fiber.Destroy();
			fiberWithState->state = FibersWorkingState::AVAILABLE;
		}


		// Reset iterattion to always execute The High priorities Job First
		i = m_Instance->m_JobQueue.size();
	}

}


bool Asfrey::AsfreyPool::FindFiber(FiberBuffer* _fiberBuffer, FiberWithState** _outFiber, FibersWorkingState _inState, FibersWorkingState _outState)
{
	for (auto& f : _fiberBuffer->fibers)
	{
		if (f.state != _inState)
		{
			continue;
		}

		if (_fiberBuffer->mutex.try_lock())
		{
			// Found a null handle 
			if (!f.fiber.HasHandle())
			{
				f.state = _outState;
				*_outFiber = &f;
				_fiberBuffer->mutex.unlock();
				return true;
			}
			_fiberBuffer->mutex.unlock();
		}

	}
	return false;
}

bool Asfrey::AsfreyPool::FindFiber(FiberPendingMap* _fiberPendingMap, FiberWithState** _outFiber, FibersWorkingState _inState, FibersWorkingState _outState)
{

	for (auto& map : _fiberPendingMap->pendingFiberMap)
	{
		for (auto& f : map.second.fibers)
		{
			if (f.state != _inState)
			{
				continue;
			}

			if (map.second.mutex.try_lock())
			{
				// Found a null handle 
				if (!f.fiber.HasHandle())
				{
					f.state = _outState;
					*_outFiber = &f;
					map.second.mutex.unlock();
					return true;
				}
				map.second.mutex.unlock();
			}

		}
	}
	return false;
}

bool Asfrey::AsfreyPool::PutInWaitList(FiberWithState** _fiberToPutInWaitList)
{
	FiberWithState* waitListAvailable = nullptr;


	if (!FindFiber(&m_Instance->m_FibersPendingMap, &waitListAvailable, FibersWorkingState::AVAILABLE, FibersWorkingState::AVAILABLE))
	{
		return false;
	}

	std::swap((**_fiberToPutInWaitList).state, waitListAvailable->state);
	Fiber::Swap((**_fiberToPutInWaitList).fiber, waitListAvailable->fiber);

	return true;
}

bool Asfrey::AsfreyPool::GetJobFromQueu(JobQueue* _JobQueue, Job* _Outjob)
{

	if (_JobQueue->queuMutex.try_lock())
	{
		Job job = *_JobQueue->queue.front();
		_JobQueue->queue.pop();
		_JobQueue->queuMutex.unlock();

		*_Outjob = job;
		return true;
	}

	return false;
}

bool Asfrey::AsfreyPool::LookForWaitingJob()
{
	for (auto& condition : m_Instance->m_Condition)
	{
		if (!condition.second)
			continue;

		FiberWithState* f = nullptr;

		bool result = FindFiber(&m_Instance->m_FibersPendingMap, &f, FibersWorkingState::WAITING, FibersWorkingState::WORKING);

		if (result)
		{
			f->fiber.Switch();
			return true;
		}
		else
		{

		}

	}

	return false;
}



#pragma once

#include <atomic>

#define ASFREY_JOB_ENTRY_POINT __stdcall

#define MAX_WORKER_THREAD 6

#define MAX_FIBER 160
namespace Asfrey
{
	using AtomicCounter = std::atomic_uint;
}
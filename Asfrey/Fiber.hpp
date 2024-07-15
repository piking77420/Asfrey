#pragma once

#if defined(__x86_64__) || defined(_M_X64)
	#define ASFREY_FIBER_X64
#else
	#error "Target architecture is not supported."
#endif

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
	#define ASFREY_FIBER_WIN
#else
	#error "Target Platform is not supported."
#endif
#include <sal.h>

typedef unsigned long long Register;

static_assert(sizeof(Register) == 8, "Incorrect register size");

namespace Asfrey 
{
	using FiberEntry = void(*)(void*);

	class Fiber
	{
	public:
		Fiber(size_t _stackSize, FiberEntry _fiberEntry, _In_opt_ void* lpParameter);
		
		Fiber() = default;

		void Create(size_t _stackSize, FiberEntry _fiberEntry, _In_opt_ void* lpParameter);

		void ConvertCurrentThreadToFiber();
		
		void ConvertCurrentFiberToThread();

		void Switch();

		void Destroy();

		static inline bool IsCurrentThreadAFiber();
		
		~Fiber();

		operator bool() const { return m_Handle != nullptr; };


		
	private:
		void* m_Handle = nullptr;
	};

}



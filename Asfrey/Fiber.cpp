#include "Fiber.hpp"

#include <stack>
#include <Windows.h>

using namespace Asfrey;


Fiber::Fiber(size_t _stackSize, FiberEntry _fiberEntry ,  _In_opt_ void* _parameter)
{
    
    m_Handle = CreateFiber(_stackSize, _fiberEntry, _parameter);
}


void Fiber::ConvertCurrentThreadToFiber()
{
    m_Handle = ConvertThreadToFiber(nullptr);
}

void Fiber::ConvertCurrentFiberToThread()
{
    ConvertCurrentFiberToThread();
}

void Fiber::Switch()
{
    SwitchToFiber(m_Handle);
}

Fiber::~Fiber()
{
    if (m_Handle != nullptr)
        DeleteFiber(m_Handle);
}


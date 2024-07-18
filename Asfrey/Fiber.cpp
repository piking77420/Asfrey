#include "Fiber.hpp"

#include <stack>
#include <Windows.h>
#include <stdexcept>

using namespace Asfrey;


void Fiber::Swap(Fiber& _left, Fiber& _right)
{
    void* tmrp = _left.m_Handle;
    _left.m_Handle = _right.m_Handle;
    _right.m_Handle = tmrp;
}

Fiber::Fiber(size_t _stackSize, FiberEntry _fiberEntry ,  _In_opt_ void* _parameter)
{   
    m_Handle = CreateFiber(_stackSize, _fiberEntry, _parameter);
}

void Asfrey::Fiber::Create(size_t _stackSize, FiberEntry _fiberEntry, void* _parameter)
{
    m_Handle = CreateFiber(_stackSize, _fiberEntry, _parameter);
}

Fiber& Asfrey::Fiber::operator=(const Fiber& other)
{
    m_Handle = other.m_Handle;
    
    return *this;
}

Fiber& Asfrey::Fiber::operator=(Fiber&& other) noexcept
{
    m_Handle = std::move(other.m_Handle);

    return *this;
}

void Fiber::ConvertCurrentThreadToFiber()
{
    if (!IsCurrentThreadAFiber())
    {
        m_Handle = ConvertThreadToFiber(nullptr);
       
        if (!m_Handle)
        {
            throw std::runtime_error("Failed to convert thread to fiber");
        }
    }
}

void Fiber::ConvertCurrentFiberToThread()
{
    if (IsCurrentThreadAFiber())
    {
        ConvertFiberToThread();
        m_Handle = nullptr;
    }
}

void Fiber::Switch()
{
    if (m_Handle != nullptr) 
    {
        SwitchToFiber(m_Handle);
    }
    else {
        throw std::runtime_error("Attempted to switch to an uninitialized fiber");
    }
}

void Asfrey::Fiber::Destroy()
{
    if (m_Handle != nullptr)
    {
        DeleteFiber(m_Handle);
        m_Handle = nullptr;
    }
}

bool Asfrey::Fiber::IsCurrentThreadAFiber()
{
    return IsThreadAFiber();
}

Fiber::~Fiber()
{
    Destroy();
}


#pragma once

#include <functional>
#include <utility>

#include "Asfrey_header.h"

namespace Asfrey
{
    using _FuncPtr = void(*)(void*);

    enum class JobPriorities
    {
        LOW,
        MEDIUM,
        HIGH,

        COUNT
    };

    enum class JobCondition
    {
        LOW,
        MEDIUM,
        HIGH,

        COUNT
    };

    struct Job
    {
        _FuncPtr func = nullptr;
        void* arg = nullptr;   
        JobPriorities jobPriorities = JobPriorities::MEDIUM;
        AtomicCounter* counter = nullptr;
       

        void operator()() const
        {
            if (func)
            {
                func(arg);
            }
        }
    };
}

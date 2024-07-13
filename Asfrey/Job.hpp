#pragma once

#include <functional>
#include <utility>

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

    struct Job
    {
        _FuncPtr func = nullptr;
        void* arg = nullptr;     

        void operator()() const
        {
            if (func)
            {
                func(arg);
            }
        }
    };
}

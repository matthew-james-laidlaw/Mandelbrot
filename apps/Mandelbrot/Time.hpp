#pragma once

#include <chrono>
#include <functional>

namespace chrono = std::chrono;

template <typename Callable, typename... Arguments>
auto Time(Callable&& callable, Arguments&&... arguments)
{
    if constexpr (std::is_void_v<std::invoke_result_t<Callable, Arguments...>>)
    {
        auto start = chrono::high_resolution_clock::now();
        std::invoke(std::forward<Callable>(callable), std::forward<Arguments>(arguments)...);
        auto end     = chrono::high_resolution_clock::now();
        auto elapsed = chrono::duration<double>(end - start);
        return elapsed;
    }
    else
    {
        auto start   = chrono::high_resolution_clock::now();
        auto result  = std::invoke(std::forward<Callable>(callable), std::forward<Arguments>(arguments)...);
        auto end     = chrono::high_resolution_clock::now();
        auto elapsed = chrono::duration<double>(end - start);
        return std::make_pair(result, elapsed);
    }
}

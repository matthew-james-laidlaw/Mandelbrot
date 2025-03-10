#pragma once

#include <ThreadPool.hpp>

template <typename Callable>
auto DispatchElement(size_t height, size_t width, Callable&& callable) -> void
{
    ThreadPool thread_pool;
    size_t num_threads = thread_pool.Threads();

    size_t num_tasks = (height < num_threads) ? height : num_threads;

    size_t rows_per_task = height / num_tasks;
    size_t remainder     = height % num_tasks;

    for (size_t t = 0; t < num_tasks; ++t)
    {
        size_t row_start = t * rows_per_task + std::min(t, remainder);
        size_t extra     = (t < remainder) ? 1 : 0;
        size_t row_end   = row_start + rows_per_task + extra;

        thread_pool.Enqueue([=, &callable]()
        {
            for (size_t y = row_start; y < row_end; ++y)
            {
                for (size_t x = 0; x < width; ++x)
                {
                    callable(y, x);
                }
            }
        });
    }

    thread_pool.Wait();
}

template <typename Callable>
auto DispatchRow(size_t height, Callable&& callable) -> void
{
    ThreadPool thread_pool;
    size_t num_threads   = thread_pool.Threads();
    size_t num_tasks     = (height < num_threads) ? height : num_threads;
    size_t rows_per_task = height / num_tasks;
    size_t remainder     = height % num_tasks;

    for (size_t t = 0; t < num_tasks; ++t)
    {
        size_t row_start = t * rows_per_task + std::min(t, remainder);
        size_t extra     = (t < remainder) ? 1 : 0;
        size_t row_end   = row_start + rows_per_task + extra;

        thread_pool.Enqueue([=, &callable]()
        {
            for (size_t y = row_start; y < row_end; ++y)
            {
                callable(y);
            }
        });
    }
    thread_pool.Wait();
}

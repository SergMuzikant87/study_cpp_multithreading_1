#pragma once
#include <atomic>
#include <chrono>

typedef enum : bool
{
    RESOURCE_IS_FREE,
    RESOURCE_IS_BUZY
}Resource_Status;

class Resource_Safe
{
public:
    Resource_Safe(std::chrono::duration<int64_t, std::micro> resource_buzy_timeout_us = std::chrono::microseconds(10000))
    {
        __resource_buzy_timeout_us = resource_buzy_timeout_us;
        __resource_status.store(RESOURCE_IS_FREE);
    }

    void resource_buzy_set(void)
    {
        Resource_Status resource_expected = RESOURCE_IS_FREE;
        bool timer_runing = false;
        auto time_us_start = std::chrono::time_point<std::chrono::steady_clock>{};
        while(!__resource_status.compare_exchange_weak(resource_expected, RESOURCE_IS_BUZY))
        {
            auto time_us_cur = std::chrono::steady_clock::now();
            if(!timer_runing)
            {
                time_us_start = time_us_cur;
                timer_runing = true;
            }
            std::chrono::duration<int64_t, std::micro> time_us_buzy = std::chrono::duration_cast<std::chrono::microseconds>(time_us_cur - time_us_start);
            if(time_us_buzy >= __resource_buzy_timeout_us)
            {
                throw std::runtime_error("ERROR: Время ожидания ресурса вышло.\n");
            }
            resource_expected = RESOURCE_IS_FREE;
        }
    }

    void resource_free_set(void)
    {
        Resource_Status resource_expected = RESOURCE_IS_BUZY;
        if(!__resource_status.compare_exchange_strong(resource_expected, RESOURCE_IS_FREE))
        {
            throw std::runtime_error("ERROR: Русурс не занят непонятно почему.\n");
        }
    }

private:
    std::atomic<Resource_Status> __resource_status;
    std::chrono::duration<int64_t, std::micro> __resource_buzy_timeout_us;
};

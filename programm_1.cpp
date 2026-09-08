#include <iostream>
#include <cstdint>
#include <thread>
#include <chrono>
#include <vector>
#include <atomic>
#include <clocale>
#include "resource_safe.h"

std::atomic<bool> is_opened;

void increment(volatile int & count, int maximum, uint32_t interval_microseconds, uint32_t opened_timeout, Resource_Safe & resource_safe)
{
    auto time_us_start = std::chrono::steady_clock::now();
    while(!is_opened.load())
    {
        auto time_us_cur = std::chrono::steady_clock::now();
        std::chrono::duration<int64_t, std::micro> interval_us = std::chrono::duration_cast<std::chrono::microseconds>(time_us_cur - time_us_start);
        if(interval_us.count() >= opened_timeout)
        {
            resource_safe.resource_buzy_set();
            std::cout << "Время ожидания открытия лавочки вышло.\n";
            resource_safe.resource_free_set();
            return;
        }
    }

    while(1)
    {
        std::this_thread::sleep_for(std::chrono::microseconds(interval_microseconds));
        resource_safe.resource_buzy_set();
        if(!is_opened.load())
        {
            std::cout << "Лавочка закрывается. Расходимся\n";
            resource_safe.resource_free_set();
            return;
        }
        count += 1;
        std::cout << "Пришёл новый клиент. Количество клиентов в очереди = " << count << "\n";
        if(count >= maximum)
        {
            std::cout << "Максимальное количество клиентов в очереди достигнуто. Обслуживаются только оставшиеся клиенты.\n";
            resource_safe.resource_free_set();
            return;
        }
        resource_safe.resource_free_set();
    }
}

void decrement(volatile int & count, uint32_t interval_microseconds, uint32_t first_client_timeout_microseconds, Resource_Safe & resource_safe)
{
    is_opened.store(true);
    resource_safe.resource_buzy_set();
    std::cout << "Лавочка открыта.\n";
    std::cout << "Ожидание, что клиенты появятся в очереди.\n";
    resource_safe.resource_free_set();

    auto time_us_start = std::chrono::steady_clock::now();
    while(1)
    {
        resource_safe.resource_buzy_set();
        if(count > 0)
        {
            std::cout << "Клиенты появились в очереди.\n";
            resource_safe.resource_free_set();
            break;
        }
        resource_safe.resource_free_set();

        auto time_us_cur = std::chrono::steady_clock::now();
        std::chrono::duration<int64_t, std::micro> interval_us = std::chrono::duration_cast<std::chrono::microseconds>(time_us_cur - time_us_start);
        if(interval_us.count() >= first_client_timeout_microseconds)
        {
            is_opened.store(false);
            resource_safe.resource_buzy_set();
            std::cout << "Время ожидания первого клиента вышло.\n";
            resource_safe.resource_free_set();
            return;
        }
    }

    while(1)
    {
        std::this_thread::sleep_for(std::chrono::microseconds(interval_microseconds));
        resource_safe.resource_buzy_set();
        count -= 1;
        if(count <= 0)
        {
            is_opened.store(false);
            std::cout << "Очередь пуста. Лавочка закрывается.\n";
            resource_safe.resource_free_set();
            return;
        }
        std::cout << "Вопрос клиента решен. Количество клиентов в очереди = " << count << "\n";
        resource_safe.resource_free_set();
    }
}

int main(int argc, char * argv [])
{
    setlocale(LC_ALL, "Russian");

    volatile int counter = 0;
    Resource_Safe resource_safe;
    std::thread thread_increment(increment, std::ref(counter), 10, 1000000, 1000000, std::ref(resource_safe));
    std::thread thread_decrement(decrement, std::ref(counter), 2000000, 5000000, std::ref(resource_safe));
    thread_increment.join();
    thread_decrement.join();
    return 0;
}

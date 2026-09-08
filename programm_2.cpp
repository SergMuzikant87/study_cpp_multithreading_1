#include <iostream>
#include <iomanip>
#include <thread>
#include <cstdint>
#include <vector>
#include <ctime>
#include <clocale>

static std::vector<uint64_t> values_random_generate(uint64_t count, uint64_t mask = 0x00000FFF);
static void values_print(const std::vector<uint64_t> & vector);

#define ELEMENTS_COUNT_MINIMUM (1000)
#define ELEMENTS_COUNT_MAXIMUM (1000000)
#define THREADS_COUNT_MAXIMUM (16)

uint64_t elements_count {};
uint64_t threads_count {};
uint64_t thread_index {};
uint64_t threads_results[THREADS_COUNT_MAXIMUM];
uint64_t thread_elements_count_maximum {};
uint64_t * threads_elements_start_pointers[THREADS_COUNT_MAXIMUM] {};
uint64_t * threads_elements_stop_pointers[THREADS_COUNT_MAXIMUM] {};
uint64_t result {};
std::vector<uint64_t> elements {};

inline void __attribute__((always_inline)) thread_job(uint64_t thread_index)
{
    register uint64_t current_thread_result = 0;
    register uint64_t * current_element_pointer = threads_elements_start_pointers[thread_index];
    register uint64_t * stop_element_pointer = threads_elements_stop_pointers[thread_index];

    while(current_element_pointer <= stop_element_pointer)
    {
        current_thread_result += *current_element_pointer++;
    }

    threads_results[thread_index] = current_thread_result;
}

int main(int argc, char * argv [])
{
    setlocale(LC_ALL, "Russian");

#if 1
    elements = values_random_generate(ELEMENTS_COUNT_MAXIMUM, 0x00000FFF);
#endif
#if 0
    values_print(elements);
#endif

    std::cout << "Количество вычислительных устройств = " << std::thread::hardware_concurrency() << "\n";
    std::cout << std::string(57, '-') << std::endl;
    std::cout << std::left << std::setw(23) << "| Элементов " << std::setw(21) << "| Потоков " << std::setw(19) << "| Сумма " << std::setw(25) << "| Время (нс)  |" << std::endl;
    std::cout << std::string(57, '-') << std::endl;

    for(elements_count = ELEMENTS_COUNT_MINIMUM; elements_count <= ELEMENTS_COUNT_MAXIMUM; elements_count *= ((uint64_t)10))
    {
#if 0
        elements = values_random_generate(elements_count, 0x00000FFF);
#endif
#if 0
        values_print(elements);
#endif
        for(uint64_t doubling_threads_count = 0; doubling_threads_count <= 4; doubling_threads_count++)
        {
            // Подготовка
            threads_count = 1 << doubling_threads_count;
            thread_elements_count_maximum = (elements_count >> doubling_threads_count);

            for( thread_index = 0; thread_index < threads_count; thread_index++)
            {
                threads_elements_start_pointers[thread_index] = &(elements[thread_index * (thread_elements_count_maximum)]);
                threads_elements_stop_pointers[thread_index] = &(elements[((thread_index != (threads_count - 1) ? (thread_index * (thread_elements_count_maximum)) + thread_elements_count_maximum - 1: elements_count - 1))]);
                threads_results[thread_index] = 0;
            }
            result = 0;
            std::thread threads[THREADS_COUNT_MAXIMUM] {};

            ///////////////////////////////////////////////////////////////////////////////////////////

            // Расчёт
            auto time_ns_start = std::chrono::high_resolution_clock::now();
            for(uint64_t thread_index = 0; thread_index < threads_count; thread_index++)
            {
                threads[thread_index] = std::thread(thread_job, thread_index);
            }

            for(uint64_t thread_index = 0; thread_index < threads_count; thread_index++)
            {
                if(threads[thread_index].joinable())
                {
                    threads[thread_index].join();
                }
                result += threads_results[thread_index];
            }
            auto time_ns_stop = std::chrono::high_resolution_clock::now();
            std::chrono::duration<int64_t, std::nano> interval_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(time_ns_stop - time_ns_start);

            //////////////////////////////////////////////////////////////////////////////////////////////
            //Вывод результатов
            std::cout << std::left << std::setw(2) << "| " << std::setw(12) << elements_count << std::setw(2) << "| " << std::setw(12) << threads_count << std::setw(2) << "| " << std::setw(12) << result << "| " << std::setw(12) << interval_ns.count() << std::setw(1) << "|" << "\n";
            std::cout << std::string(57, '-') << std::endl;
        }
    }
    return 0;
}

static std::vector<uint64_t> values_random_generate(uint64_t count, uint64_t mask)
{
    std::vector<uint64_t> vector {};

    if(count == 0)
    {
        return vector;
    }

#if 0
    while(count--)
    {
        vector.push_back(1ULL);
    }
#elif 0
    for(uint64_t value = 1ULL; value <= count; value++)
    {
        vector.push_back(value);
    }
#else

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wparentheses"
#pragma GCC diagnostic ignored "-Wuninitialized"
    for(uint64_t index = 0; index < count; index++)
    {
        static uint64_t value_prev {0xAB581A35};
        uint64_t value = value_prev + value ^ ((uint64_t)(clock() & 0x000000FF) << 24) ^ (((uint64_t)(clock())) >> 8) ^ value_prev;
        value_prev = value;
        vector.push_back(value);
    }

    for(uint64_t index = 0; index < vector.size(); index++)
    {
        vector[index] = ( ((vector[index] ^ vector[vector.size() - index - 1]) + (vector[0]/3))  %  789 ) & mask;
    }
#pragma GCC diagnostic pop
#pragma GCC diagnostic pop

#endif
    return vector;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
static void values_print(const std::vector<uint64_t> & vector)
{
    for(uint64_t index = 0; index < vector.size(); index++)
    {
        std::cout << vector[index] << (((index & 0x1F) == 0x1F) ? "\n" : " ");
    }
    std::cout << "\n";
}
#pragma GCC diagnostic pop

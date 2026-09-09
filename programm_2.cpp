#include <iostream>
#include <iomanip>
#include <thread>
#include <cstdint>
#include <vector>
#include <ctime>
#include <clocale>

static std::vector<uint64_t> values_random_generate(uint64_t count, uint64_t mask = 0x00000FFF);
static void values_print(const std::vector<uint64_t> & vector_1, const std::vector<uint64_t> & vector_2, const std::vector<uint64_t> & vector_summ_vectors_1_2);
static bool values_check(const std::vector<uint64_t> & vector_1, const std::vector<uint64_t> & vector_2, const std::vector<uint64_t> & vector_summ_vectors_1_2);

#define ITERS_COUNT_DEFAULT (1000)
#define ELEMENTS_COUNT_MINIMUM (1000)
#define ELEMENTS_COUNT_MAXIMUM (1000000)
#define THREADS_COUNT_MAXIMUM (16)

uint64_t elements_count {};
uint64_t threads_count {};
uint64_t thread_index {};
uint64_t thread_elements_count_maximum {};
uint64_t * threads_elements_1_start_pointers[THREADS_COUNT_MAXIMUM] {};
uint64_t * threads_elements_2_start_pointers[THREADS_COUNT_MAXIMUM] {};
uint64_t * threads_elements_summ_start_pointers[THREADS_COUNT_MAXIMUM] {};
uint64_t * threads_elements_summ_stop_pointers[THREADS_COUNT_MAXIMUM] {};

typedef enum : uint64_t
{
    FIRST_ITER_TIME_COLUMN,
    MINIMUM_TIME_COLUMN,
    MAXIMUM_TIME_COLUMN,
    MIDDLE_TIME_COLUMN,
    COLUMS_COUNT
}result_colums;

int64_t results_row[COLUMS_COUNT] {};
int64_t interval_ns {};

uint64_t iters_count {};
uint64_t iter {};

std::vector<uint64_t> elements_1 {};
std::vector<uint64_t> elements_2 {};
std::vector<uint64_t> elements_summ {};

inline void __attribute__((always_inline)) thread_job(uint64_t thread_index)
{
    register uint64_t * elements_1_current_pointer = threads_elements_1_start_pointers[thread_index];
    register uint64_t * elements_2_current_pointer = threads_elements_2_start_pointers[thread_index];
    register uint64_t * elements_summ_current_pointer = threads_elements_summ_start_pointers[thread_index];
    register uint64_t * elements_summ_stop_pointer = threads_elements_summ_stop_pointers[thread_index];
    while(elements_summ_current_pointer <= elements_summ_stop_pointer)
    {
        *elements_summ_current_pointer++ = (*elements_1_current_pointer++) + (*elements_2_current_pointer++);
    }
}

int main(int argc, char * argv [])
{
    setlocale(LC_ALL, "Russian");

    iters_count = ITERS_COUNT_DEFAULT;

    std::cout << "Создаётся массив 1\n";
    elements_1 = values_random_generate(ELEMENTS_COUNT_MAXIMUM, 0x00000FFF);
    std::cout << "Создаётся массив 2\n";
    elements_2 = values_random_generate(ELEMENTS_COUNT_MAXIMUM, 0x00000FFF);
    std::cout << "Создаётся массив результатов сложения массивов 1 и 2\n";
    elements_summ = values_random_generate(ELEMENTS_COUNT_MAXIMUM, 0x00000000);

    std::cout << "Количество выполняемых итераций = " << iters_count << "\n";
    std::cout << "Количество вычислительных устройств = " << std::thread::hardware_concurrency() << "\n";
    std::cout << "Результаты:\n";
    std::cout << std::string(97, '-') << std::endl;
    std::cout << "|               |               |              Время выполнения (наносекунд)                    |\n";
    std::cout << "|  Элементов    |    Потоков    |---------------------------------------------------------------|\n";
    std::cout << "|               |               | за 1-й проход |    минимум    |   максимум    |    среднее    |\n";
    std::cout << std::string(97, '-') << std::endl;

    for(elements_count = ELEMENTS_COUNT_MINIMUM; elements_count <= ELEMENTS_COUNT_MAXIMUM; elements_count *= ((uint64_t)10))
    {
        for(uint64_t doubling_threads_count = 0; doubling_threads_count <= 4; doubling_threads_count++)
        {
            // Подготовка
            threads_count = 1 << doubling_threads_count;
            thread_elements_count_maximum = (elements_count >> doubling_threads_count);

            for(thread_index = 0; thread_index < threads_count; thread_index++)
            {
                static uint64_t index_start {};
                static uint64_t index_stop {};
                index_start = thread_index * (thread_elements_count_maximum);
                index_stop = ((thread_index != (threads_count - 1) ? index_start + thread_elements_count_maximum - 1: elements_count - 1));
                threads_elements_1_start_pointers[thread_index] = &(elements_1[index_start]);
                threads_elements_2_start_pointers[thread_index] = &(elements_2[index_start]);
                threads_elements_summ_start_pointers[thread_index] = &(elements_summ[index_start]);
                threads_elements_summ_stop_pointers[thread_index] = &(elements_summ[index_stop]);
            }

            for(iter = 0; iter < iters_count; iter++)
            {
                std::thread threads[THREADS_COUNT_MAXIMUM] {};
                ///////////////////////////////////////////////////////////////////////////////////////////

                // Расчёт

                auto time_ns_start = std::chrono::high_resolution_clock::now();
                for(thread_index = 1; thread_index < threads_count; thread_index++)
                {
                    threads[thread_index] = std::thread(thread_job, thread_index);
                }
                thread_job(0);

                for(thread_index = 1; thread_index < threads_count; thread_index++)
                {
                    if(threads[thread_index].joinable())
                    {
                        threads[thread_index].join();
                    }
                }

                auto time_ns_stop = std::chrono::high_resolution_clock::now();
                interval_ns = std::chrono::duration<int64_t, std::nano>(std::chrono::duration_cast<std::chrono::nanoseconds>(time_ns_stop - time_ns_start)).count();

                if(iter == 0)
                {
                    results_row[FIRST_ITER_TIME_COLUMN] = interval_ns;
                    results_row[MINIMUM_TIME_COLUMN] = interval_ns;
                    results_row[MAXIMUM_TIME_COLUMN] = interval_ns;
                    results_row[MIDDLE_TIME_COLUMN] = 0;
                }

                if(interval_ns < results_row[MINIMUM_TIME_COLUMN])
                {
                    results_row[MINIMUM_TIME_COLUMN] = interval_ns;
                }

                if(interval_ns > results_row[MAXIMUM_TIME_COLUMN])
                {
                    results_row[MAXIMUM_TIME_COLUMN] = interval_ns;
                }

                results_row[MIDDLE_TIME_COLUMN] += interval_ns;

            }
            results_row[MIDDLE_TIME_COLUMN] /= iters_count;
            //////////////////////////////////////////////////////////////////////////////////////////////

            //Вывод результатов
            std::cout << std::left << "| " << std::setw(14) << elements_count
                                   << "| " << std::setw(14) << threads_count
                                   << "| " << std::setw(14) << results_row[FIRST_ITER_TIME_COLUMN]
                                   << "| " << std::setw(14) << results_row[MINIMUM_TIME_COLUMN]
                                   << "| " << std::setw(14) << results_row[MAXIMUM_TIME_COLUMN]
                                   << "| " << std::setw(14) << results_row[MIDDLE_TIME_COLUMN]
                                   << "|" << "\n";
            std::cout << std::string(97, '-') << std::endl;
        }
    }

    std::cout << "Проверка посчитанной суммы массивов: ";
    if(!values_check(elements_1, elements_2, elements_summ))
    {
        std::cout << "ERROR: Некорректно посчитана сумма массивов.\n";
        return 1;
    }
    std::cout << "Всё в норме.\n";

#if 0
    values_print(elements_1, elements_2, elements_summ);
#endif

    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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


static bool values_check(const std::vector<uint64_t> & vector_1, const std::vector<uint64_t> & vector_2, const std::vector<uint64_t> & vector_summ_vectors_1_2)
{
    for(uint64_t index = 0; index < vector_summ_vectors_1_2.size(); index++)
    {
        if((vector_1[index] + vector_2[index]) != vector_summ_vectors_1_2[index])
        {
            return false;
        }
    }
    return true;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
static void values_print(const std::vector<uint64_t> & vector_1, const std::vector<uint64_t> & vector_2, const std::vector<uint64_t> & vector_summ_vectors_1_2)
{
    for(uint64_t index = 0; index < vector_summ_vectors_1_2.size(); index++)
    {
        std::cout << vector_1[index] << " + " << vector_1[index] << " = "<< vector_summ_vectors_1_2[index] << (((index & 0x07) == 0x07) ? "\n" : ";  ");
    }
    std::cout << "\n";
}
#pragma GCC diagnostic pop

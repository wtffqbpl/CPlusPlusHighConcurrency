#include <thread>
#include <mutex>
#include <iostream>
#include <vector>

#define MAX_TRIED_TIMES   (10000)
#define MAX_THREAD_NUM    (10)

volatile int counter(0); // non-atomic counter
std::mutex mtx;          // locks access to counter


void attempt_10k_increases()
{
    for (int timesIdx = 0; timesIdx < MAX_TRIED_TIMES; ++timesIdx) {
        // if (mtx.lock()) {
#if 0
        if (mtx.lock()) {
            ++counter;
            mtx.unlock();
        }
#endif
        mtx.lock();
        ++counter;
        mtx.unlock();
    }
}

/*********************************************************************************
 *
 *     NAME:   main
 *  Description: program entry routine.
 *
 *********************************************************************************/
int main(int argc, char* argv[])
{

    std::vector<std::thread> threads;
    threads.reserve(MAX_THREAD_NUM);


    // std::thread threads[MAX_THREAD_NUM];

    for (int threadIdx = 0; threadIdx < MAX_THREAD_NUM; ++threadIdx) {
        // threads[threadIdx] = std::thread(attempt_10k_increases);
        std::cout << "Create " << threadIdx << "-th thread.\n";
        threads.push_back(std::thread(attempt_10k_increases));
    }

    std::cout << "Join all threads.\n";
    for (auto& th: threads) {
        th.join();
    }

    std::cout << counter << " successful increases of the counter." << std::endl;

    return 0;
}

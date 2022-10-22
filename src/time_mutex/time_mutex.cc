#include <gtest/gtest.h>
#include <thread>
#include <mutex>
#include <chrono>
#include <iostream>
#include <vector>

namespace {

#define MAX_THREAD_NUM    (10)

std::timed_mutex mtx;          // locks access to counter


void fireworks()
{
    // waiting to get a lock: each thread prints "-" every 20ms.
    while (!mtx.try_lock_for(std::chrono::milliseconds(200))) {
        std::cout << "-";
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::cout << "*\n";
    mtx.unlock();
}

}

TEST(time_mutex, demo) {
    std::vector<std::thread> threads;
    threads.reserve(MAX_THREAD_NUM);

    for (int thIdx = 0; thIdx < MAX_THREAD_NUM; ++thIdx) {
        threads.push_back(std::thread(fireworks));
    }

    for (auto& th: threads) th.join();
}
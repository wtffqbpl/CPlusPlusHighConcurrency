#include <gtest/gtest.h>
#include <thread>
#include <mutex>
#include <iostream>
#include <vector>
#include <shared_mutex>

namespace {

constexpr int MAX_TRIED_TIMES = 10000;
constexpr int MAX_THREAD_NUM = 10;

int counter(0); // non-atomic counter
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

using namespace std::chrono_literals;

std::mutex g_mutex;

void thread_func() {
    g_mutex.lock();

    std::cout << "Thread out 1: " << std::this_thread::get_id() << std::endl;
    std::this_thread::sleep_for(1s);
    std::cout << "Thread out 2: " << std::this_thread::get_id() << std::endl;
    g_mutex.unlock();
}

void test_mutex() {
    std::cout << "Mutex Test." << std::endl;
    std::thread thread1(thread_func);
    std::thread thread2(thread_func);

    thread1.join();
    thread2.join();
}

std::recursive_mutex g_recursive_mutex;

void thread_func_recursive_lock(int thread_id, int time) {
    g_recursive_mutex.lock();
    std::cout << "Thread " << thread_id << ": " << time << std::endl;
    if (time != 0)
        thread_func_recursive_lock(thread_id, time - 1);
    g_recursive_mutex.unlock();
}

void test_recursive_mutex()
{
    std::thread thread1(thread_func_recursive_lock, 1, 3);
    std::thread thread2(thread_func_recursive_lock, 2, 4);

    // Wait
    thread1.join();
    thread2.join();
}

// Shared mutex

std::shared_mutex g_shared_mutex;

void thread_read_1_func(int thread_id) {
    g_shared_mutex.lock_shared();

    std::cout << "Read thread " << thread_id << " out 1." << std::endl;
    // 睡眠2s，等待读线程2，获取读权限，确认可以多个线程进行读加锁
    std::this_thread::sleep_for(2s);
    std::cout << "read thread " << thread_id << " out 2." << std::endl;

    // Release shared mutex
    g_shared_mutex.unlock_shared();
}

void thread_read_2_func(int thread_id) {
    std::this_thread::sleep_for(500ms);
    g_shared_mutex.lock_shared();
    std::cout << "Read thread " << thread_id << " out 1." << std::endl;
    std::this_thread::sleep_for(3s);
    std::cout << "Read thread " << thread_id << " out 2." << std::endl;
    g_shared_mutex.unlock_shared();
}

void thread_write_1_func(int thread_id) {
    std::this_thread::sleep_for(300ms);
    g_shared_mutex.lock();
    std::cout << "Write thread " << thread_id << " out 1." << std::endl;
    g_shared_mutex.unlock();
}

}

TEST(mutex_demo, basic_test) {
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
}

TEST(mutex_demo, test2) {
    test_mutex();
}

TEST(mutex_demo, test3) {
    test_recursive_mutex();
}
#include "gtest/gtest.h"
#include <chrono>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <vector>

constexpr size_t max_thread_num = 10;

std::mutex mtx;          // locks access to counter

void print_even(int x) {
    if (x % 2 == 0) {
        std::cout << x << " is even\n";
    } else {
        throw (std::logic_error("not even"));
    }
}

void print_thread_id(int id) {
    try {
        // using a local lock_guard to lock mtx guarantees unlocking on destruction / exception.
        std::lock_guard<std::mutex> lck(mtx);
        print_even(id);
    }
    catch(std::logic_error&) {
        std::cout << "[exception caught]\n";
    }
}

/*********************************************************************************
 *
 *     NAME:   main
 *  Description: program entry routine.
 *
 *********************************************************************************/
TEST(lock_guard, demo) {
  std::vector<std::thread> threads;
  threads.reserve(max_thread_num);

  for (size_t threadIdx = 0; threadIdx < max_thread_num; ++threadIdx)
    threads.emplace_back(std::thread(print_thread_id, threadIdx+1));

  for (auto& th: threads) th.join();
}
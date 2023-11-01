#include "gtest/gtest.h"
#include <algorithm>
#include <iostream>
#include <list>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <vector>

namespace {

constexpr size_t max_thread_num = 10;

std::mutex mtx; // locks access to counter

void print_even(int x) {
  if (x % 2 == 0) {
    std::cout << x << " is even\n";
  } else {
    throw(std::logic_error("not even"));
  }
}

void print_thread_id(int id) {
  try {
    // using a local lock_guard to lock mtx guarantees unlocking on destruction / exception.
    std::lock_guard<std::mutex> lck(mtx);
    print_even(id);
  } catch (std::logic_error &) {
    std::cout << "[exception caught]\n";
  }
}

std::list<int> some_list;
std::mutex some_mutex;

void add_to_list(int new_value) {
  std::lock_guard<std::mutex> guard(some_mutex);
  some_list.push_back(new_value);
}

bool list_contains(int value_to_find) {
  std::lock_guard<std::mutex> guard(some_mutex);

  return std::find(some_list.begin(), some_list.end(), value_to_find)
      != some_list.end();
}

volatile int g_i = 0;
std::mutex g_i_mutex; // protects g_i;

void safe_increment(int iterations) {
  const std::lock_guard<std::mutex> lock(g_i_mutex);
  while (iterations-- > 0)
    g_i = g_i + 1;
  std::cout << "thread #" << std::this_thread::get_id() << ", g_i: " << g_i << '\n';
  std::flush(std::cout);
  // g_i_mutex is automatically released when lock goes out of scope
}

void unsafe_increment(int iterations) {
  while (iterations-- > 0)
    g_i = g_i + 1;
  std::cout << "thread #" << std::this_thread::get_id() << ", g_i: " << g_i << '\n';
  std::flush(std::cout);
}

void lock_guard_test2() {
  auto test = [](std::string_view fun_name, auto fun) {
    g_i = 0;
    std::cout << fun_name << ":\nbefore, g_i: " << g_i << '\n';

    std::thread t1(fun, 1'000'000);
    std::thread t2(fun, 1'000'000);

    if (t1.joinable())
      t1.join();
    if (t2.joinable())
      t2.join();

    std::cout << "after, g_i: " << g_i << "\n\n";
  };

  test("safe_increment", safe_increment);
  test("unsafe_increment", unsafe_increment);
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
    threads.emplace_back(print_thread_id, threadIdx+1);

  for (auto& th: threads) th.join();
}

TEST(lock_guard_test, test2) {
  lock_guard_test2();
}
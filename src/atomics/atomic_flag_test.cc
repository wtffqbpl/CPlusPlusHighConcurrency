#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <gtest/gtest.h>

namespace {
std::atomic_flag g_lock = ATOMIC_FLAG_INIT;

static bool  g_running = false;
static int g_data = 0;

void thread_function() {
  std::cout << "thread_function running... " << std::endl;
  {
    while (g_lock.test_and_set());
    g_data += 10;
    std::cout << "g_data = " << g_data << std::endl;
    std::cout << "[" << __func__ << "]" << std::endl;
    g_lock.clear();
  }
}

void atomic_flag_test()
{
  int data = 10;
  std::thread thread1(thread_function);

  std::cout << "main thread running..." << std::endl;

  if (thread1.joinable())
    thread1.join();
}

std::atomic<bool> g_going2(false);
std::atomic_flag g_lock2 = ATOMIC_FLAG_INIT;

int g_data2 = 0;
bool g_running2 = false;

void thread_function_2() {
  std::cout << "thread_function_2 running..." << std::endl;

  while (g_running2) {
    {
      while (!g_going2);
      if (g_lock2.test_and_set()) {
        g_data2 += 10;
        std::cout << "g_data = " << g_data << std::endl;
        std::cout << "[" << __func__ << "]" << std::endl;
        g_lock2.clear();
        g_going2 = false;
      }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  std::cout << "thread_function_2 exit..." << std::endl;
}

void test_atomic_flag_2() {
  int data = 10;
  g_running2 = true;
  std::thread thread1(thread_function_2);

  std::cout << "main thread running..." << std::endl;

  for (int i = 0; i < 10; ++i) {
    g_going2 = true;
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }

  g_running2 = false;
  g_going2 = true;

  if (thread1.joinable())
    thread1.join();
}

// Spin lock implementation with std::atomic_flag
class SpinLockMutex {
  std::atomic_flag flag;

public:
  SpinLockMutex() : flag(ATOMIC_FLAG_INIT)
  {}

  void lock() {
    while (flag.test_and_set(std::memory_order_acquire));
  }

  void unlock() {
    flag.clear(std::memory_order_release);
  }
};

}

TEST(atomic_flag_test, test1) {
  atomic_flag_test();
}

TEST(atomic_flag_test, test2) {
  test_atomic_flag_2();
}
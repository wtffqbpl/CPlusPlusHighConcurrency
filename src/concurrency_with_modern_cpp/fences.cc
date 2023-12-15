#include <gtest/gtest.h>

#include <atomic>
#include <thread>
#include <iostream>
#include <string>
#include <cassert>
#include <csignal>

namespace memory_order_utils {

std::atomic<std::string*> ptr;
int data;
std::atomic<int> a_to_data;

void producer() {
  auto *p = new std::string("C++11");
  data = 2011;
  a_to_data.store(2014, std::memory_order_release);
  ptr.store(p, std::memory_order_release);
}

void consumer() {
  std::string *p2;
  while (!(p2 = ptr.load(std::memory_order_acquire)));
  std::cout << "*p2: " << *p2 << std::endl;
  std::cout << "data: " << data << std::endl;
  std::cout << "a_to_data: " << a_to_data.load(std::memory_order_acquire) << std::endl;
}

}

TEST(fences_test, test1) {
  std::cout << std::endl;

  std::thread t1(memory_order_utils::producer);
  std::thread t2(memory_order_utils::consumer);

  t1.join();
  t2.join();

  std::cout << std::endl;
}

namespace fences_utils {

std::atomic<std::string*> ptr;
int data;
std::atomic<int> a_to_data;

void producer() {
  auto *p = new std::string("C++11");
  data = 2011;
  a_to_data.store(2014, std::memory_order_relaxed);
  std::atomic_thread_fence(std::memory_order_release);
  ptr.store(p, std::memory_order_relaxed);
}

void consumer() {
  std::string *p2;
  while (!(p2 = ptr.load(std::memory_order_relaxed)));
  std::atomic_thread_fence(std::memory_order_acquire);

  std::cout << "*p2: " << *p2 << std::endl;
  std::cout << "data: " << data << std::endl;
  std::cout << "a_to_data: " << a_to_data.load(std::memory_order_relaxed) << std::endl;
}

}

TEST(fences_test, test2) {
  std::cout << std::endl;

  std::thread t1(fences_utils::producer);
  std::thread t2(fences_utils::consumer);

  t1.join();
  t2.join();

  std::cout << std::endl;
}

namespace signal_fence_utils {

std::atomic<bool> a{false};
std::atomic<bool> b{false};

extern "C" void handler(int) {
  if (a.load(std::memory_order_relaxed)) {
    std::atomic_signal_fence(std::memory_order_acquire);
    assert(b.load(std::memory_order_relaxed));
  }
}

}

TEST(fences_test, signal_fence_test1) {
  std::signal(SIGTERM, signal_fence_utils::handler);

  signal_fence_utils::b.store(true, std::memory_order_relaxed);
  std::atomic_signal_fence(std::memory_order_release);
  signal_fence_utils::a.store(true, std::memory_order_relaxed);
}

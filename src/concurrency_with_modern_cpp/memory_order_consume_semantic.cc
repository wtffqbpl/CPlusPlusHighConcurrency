#include <gtest/gtest.h>
#include <atomic>
#include <thread>
#include <string>

namespace {
std::atomic<std::string*> ptr;
int data;
std::atomic<int> a_to_data;

void producer() {
  auto *p = new std::string("C++11");
  data = 2011;
  a_to_data.store(2014, std::memory_order_relaxed);
  ptr.store(p, std::memory_order_release);
}

void consumer() {
  std::string *p2;

  while (!(p2 = ptr.load(std::memory_order_acquire)));
  std::cout << "*p2: " << *p2 << std::endl;
  std::cout << "data: " << data << std::endl;
  std::cout << "a_to_data: " << a_to_data.load(std::memory_order_relaxed) << std::endl;
}

}

TEST(memory_order_consume_test, test1) {
  std::thread t1(producer);
  std::thread t2(consumer);

  t1.join();
  t2.join();
}
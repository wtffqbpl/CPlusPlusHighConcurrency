#include <gtest/gtest.h>

#include <atomic>
#include <thread>

namespace {

class Spinlock {
  std::atomic_flag flag;

public:
  Spinlock() : flag(ATOMIC_FLAG_INIT) {}

  void lock() { while (flag.test_and_set(std::memory_order_acquire)); }
  void unlock() { flag.clear(std::memory_order_release); }
};

Spinlock spin;

void work_on_resource() {
  spin.lock();
  // Shared resource
  spin.unlock();
}

}

TEST(acquire_release_test, test1) {
  std::thread t1(work_on_resource);
  std::thread t2(work_on_resource);
  t1.join();
  t2.join();
}

namespace {

std::vector<int> my_shared_work;
std::atomic<bool> data_produced(false);
std::atomic<bool> data_consumed(false);

void data_producer() {
  my_shared_work = {1, 0, 3};
  data_produced.store(true, std::memory_order_release);
}

void delivery_boy() {
  while (!data_produced.load(std::memory_order_acquire));
  data_consumed.store(true, std::memory_order_release);
}

void data_consumer() {
  while (!data_consumed.load(std::memory_order_acquire));
  my_shared_work[1] = 2;
}

}

TEST(acquire_release_test, transitivity_test) {
  std::cout << std::endl;

  std::thread t1(data_consumer);
  std::thread t2(delivery_boy);
  std::thread t3(data_producer);

  t1.join();
  t2.join();
  t3.join();

  for (auto v : my_shared_work)
    std::cout << v << ' ';

  std::cout << '\n';
}
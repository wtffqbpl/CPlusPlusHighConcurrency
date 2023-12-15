#include <gtest/gtest.h>
#include <atomic>
#include <vector>
#include <thread>

namespace {
std::atomic<int> count = {0};

void add() {
  for (int n = 0; n < 1000; ++n) {
    count.fetch_add(1, std::memory_order_relaxed);
  }
}

}

TEST(memory_order_relaxed_test, counter_test) {
  std::vector<std::thread> pool;

  for (int n = 0; n < 10; ++n)
    pool.emplace_back(add);

  for (auto &t : pool)
    t.join();

#ifndef NDEBUG
  std::cout << "Final counter value is: " << count << '\n';
#endif

  ASSERT_EQ(count, 10000);
}
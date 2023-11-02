#include <gtest/gtest.h>
#include <chrono>
#include <iostream>
#include <thread>

namespace {

// "busy sleep" while suggesting that other threads run for a
// small amount of time.
void little_sleep(std::chrono::microseconds us) {
  auto start = std::chrono::high_resolution_clock::now();
  auto end = start + us;

  do {
    std::this_thread::yield();
  }
  while (std::chrono::high_resolution_clock::now() < end);
}

void yield_test() {
  auto start = std::chrono::high_resolution_clock::now();

  little_sleep(std::chrono::microseconds(100));

  auto elapsed = std::chrono::high_resolution_clock::now() - start;

  std::cout << "waited for "
            << std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count()
            << " microseconds\n";
}

}

TEST(yield_test, test1) {
  yield_test();
}
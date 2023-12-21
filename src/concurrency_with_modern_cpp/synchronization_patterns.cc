#include <gtest/gtest.h>
#include <functional>
#include <iostream>
#include <string>
#include <thread>

namespace data_race_with_reference_utils {

using namespace std::chrono_literals;

void by_copy(bool b) {
  std::this_thread::sleep_for(1ms);
  std::cout << "by_copy: " << b << std::endl;
}

void by_reference(bool &b) {
  std::this_thread::sleep_for(1ms);
  std::cout << "by_reference: " << b << std::endl;
}

void by_const_reference(const bool &b) {
  std::this_thread::sleep_for(1ms);
  std::cout << "by_const_reference: " << b << std::endl;
}

}

TEST(synchronization_patterns, test1) {
  using namespace data_race_with_reference_utils;

  bool shared{false};

  std::thread t1(by_copy, shared);
  std::thread t2(by_reference, std::ref(shared));
  std::thread t3(by_const_reference, std::cref(shared));

  shared = true;

  t1.join();
  t2.join();
  t3.join();
}
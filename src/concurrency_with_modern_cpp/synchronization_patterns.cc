#include <gtest/gtest.h>
#include <functional>
#include <iostream>
#include <mutex>
#include <new>
#include <string>
#include <thread>
#include <utility>

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

namespace scoped_lock_utils {

class ScopedLock {
private:
  std::mutex &mut;

public:
  explicit ScopedLock(std::mutex &m) : mut(m) {
    mut.lock();
    std::cout << "Lock the mutex: " << &mut << std::endl;
  }

  ~ScopedLock() {
    std::cout << "Release the mutex: " << &mut << std::endl;
    mut.unlock();
  }
};

void test_scoped_lock() {
  std::mutex mutex1;
  ScopedLock scoped_lock1{mutex1};

  std::cout << "\nBefore local scope" << std::endl;
  {
    std::mutex mutex2;
    ScopedLock scoped_lock2{mutex2};
  }
  std::cout << "After local scope" << std::endl;

  std::cout << "\nBefore try-catch block" << std::endl;
  try {
    std::mutex mutex3;
    ScopedLock scoped_lock3{mutex3};
    throw std::bad_alloc();
  }
  catch (std::bad_alloc &e) {
    std::cout << e.what();
  }
  std::cout << "\nAfter try-catch block" << std::endl;
}

} // namespace scoped_lock_utils

TEST(synchronization_patterns, test2) {
  using namespace scoped_lock_utils;
  test_scoped_lock();
}
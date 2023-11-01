#include <gtest/gtest.h>
#include <iostream>
#include <mutex>
#include <thread>

namespace {

class hierarchical_mutex {
  std::mutex internal_mutex;
  unsigned long const hierarchy_value;
  unsigned long previous_hierarchy_value;
  static inline thread_local unsigned long this_thread_hierarchy_value = std::numeric_limits<unsigned long>::max();

  void check_for_hierarchy_violation() {
    if (this_thread_hierarchy_value <= hierarchy_value)
      throw std::logic_error("mutex hierarchy violated");
  }

  void update_hierarchy_value() {
    previous_hierarchy_value = this_thread_hierarchy_value;
    this_thread_hierarchy_value = hierarchy_value;
  }

public:
  explicit hierarchical_mutex(unsigned long value)
      : hierarchy_value(value)
      , previous_hierarchy_value(0)
  {}

  void lock() {
    check_for_hierarchy_violation();
    internal_mutex.lock();
    update_hierarchy_value();
  }

  void unlock() {
    this_thread_hierarchy_value = previous_hierarchy_value;
    internal_mutex.unlock();
  }

  bool try_lock() {
    check_for_hierarchy_violation();
    if (!internal_mutex.try_lock())
      return false;
    update_hierarchy_value();
    return true;
  }
};

hierarchical_mutex high_level_mutex(10000);
hierarchical_mutex low_level_mutex(5000);

int do_low_level_stuff() {
  std::cout << "do_low_level_stuff\n";
  std::flush(std::cout);
  return 0;
}

int low_level_func() {
  std::lock_guard<hierarchical_mutex> lk(low_level_mutex);
  return do_low_level_stuff();
}

void high_level_stuff(int some_param) {
  std::cout << "high_level_stuff: " << some_param << '\n';
  std::flush(std::cout);
}

void high_level_func() {
  std::lock_guard<hierarchical_mutex> lk(high_level_mutex);
  high_level_stuff(low_level_func());
}

void thread_a() {
  high_level_func();
}

hierarchical_mutex other_mutex(100);
void do_other_stuff() {
  std::cout << "do_other_stuff\n";
  std::flush(std::cout);
}

void other_stuff() {
  high_level_func();
  do_other_stuff();
}

void thread_b() {
  std::lock_guard<hierarchical_mutex> lk(other_mutex);
  other_stuff();
}

void hierarchical_mutex_test1() {
  std::thread tha(thread_a);
  std::thread thb(thread_b);

  if (tha.joinable()) tha.join();
  if (thb.joinable()) thb.join();
}

}

TEST(hierarchical_mutex_test, test1) {
  hierarchical_mutex_test1();
}
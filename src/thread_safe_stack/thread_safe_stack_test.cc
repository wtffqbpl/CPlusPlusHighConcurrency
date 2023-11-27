#include <gtest/gtest.h>
#include <exception>
#include <memory>
#include <mutex>
#include <stack>
#include <thread>

namespace {
struct empty_stack : std::exception
{
  [[nodiscard]] const char* what() const throw();
};

template <typename T>
class ThreadSafeStack {
private:
  std::stack<T> data;
  mutable std::mutex m;

public:
  ThreadSafeStack() = default;

  ThreadSafeStack(const ThreadSafeStack &other) {
    std::lock_guard<std::mutex> lock(other.m);
    data = other.data;
  }

  ThreadSafeStack &operator=(const ThreadSafeStack &) = delete;

  void push(T new_value) {
    std::lock_guard<std::mutex> lock(m);
    data.push(new_value);
  }

  std::shared_ptr<T> pop() {
    std::lock_guard<std::mutex> lock(m);
    if (data.empty()) throw empty_stack();
    std::shared_ptr<T> const res(std::make_shared<T>(data.pop()));
    data.pop();
    return res;
  }

  void pop(T &value) {
    std::lock_guard<std::mutex> lock(m);
    if (data.empty()) throw empty_stack();
    value = data.top();
    data.pop();
  }

  bool empty() const {
    std::lock_guard<std::mutex> lock(m);
    return data.empty();
  }
};

class some_big_object {};
void swap(some_big_object &lhs, some_big_object &rhs) {}

class X {
private:
  some_big_object some_detail;
  std::mutex m;

public:
  explicit X(some_big_object const &sd) : some_detail(sd) {}

  friend void swap(X &lhs, X &rhs) {
    if (&lhs == &rhs)
      return;

    std:lock(lhs.m, rhs.m);
    std::lock_guard<std::mutex> lock_a(lhs.m, std::adopt_lock);
    std::lock_guard<std::mutex> lock_b(rhs.m, std::adopt_lock);

    swap(lhs.some_detail, rhs.some_detail);
  }
};

class hierarchical_mutex {
public:
  using size_type = unsigned long;

  explicit hierarchical_mutex(size_type val)
      : hierarchical_value_(val)
      , previous_hierarchical_value_(0)
  {}

  void lock() {
    check_for_hierarchical_violation();
    internal_mutex_.lock();
    update_hierarchical_value();
  }

  void unlock() {
    this_thread_hierarchical_value_ = previous_hierarchical_value_;
    internal_mutex_.unlock();
  }

private:
  void check_for_hierarchical_violation() const {
    if (this_thread_hierarchical_value_ <= hierarchical_value_)
      throw std::logic_error{"mutex hierarchical violated"};
  }

  void update_hierarchical_value() {
    previous_hierarchical_value_ = this_thread_hierarchical_value_;
    this_thread_hierarchical_value_ = hierarchical_value_;
  }

private:
  std::mutex internal_mutex_;
  size_type hierarchical_value_;
  size_type previous_hierarchical_value_;

  static inline thread_local size_type this_thread_hierarchical_value_{ULONG_MAX};
};

hierarchical_mutex high_level_mutex{10000};
hierarchical_mutex low_level_mutex{5000};

int do_low_level_stuff() {
  printf("working on low level stuff\n");
  return 42;
}

int low_level_func() {
  return do_low_level_stuff();
}

void high_level_stuff(int some_param) {
  printf("do high level stuff %s\n", std::to_string(some_param).c_str());
}

void high_level_func() {
  std::lock_guard<hierarchical_mutex> lk{high_level_mutex};
  high_level_stuff(low_level_func());
}

hierarchical_mutex other_mutex{100};

void do_other_stuff() {
  printf("doing other stuff\n");
}

void other_stuff() {
  high_level_func();
  do_other_stuff();
}

}

TEST(hierarchical_mutex_test_11, test1) {
#if 0
  std::thread thread_fine{
    [] {
      high_level_func();
    }
  };

  std::thread thread_wild{
      [] {
        std::lock_guard<hierarchical_mutex> lk{other_mutex};
        other_stuff();
      }
  };

  thread_fine.join();
  thread_wild.join();
#endif
}
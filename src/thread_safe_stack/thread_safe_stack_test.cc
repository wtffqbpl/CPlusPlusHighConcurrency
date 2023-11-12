#include <gtest/gtest.h>
#include <exception>
#include <memory>
#include <mutex>
#include <stack>

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

}
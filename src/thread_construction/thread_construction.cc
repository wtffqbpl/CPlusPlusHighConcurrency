#include <gtest/gtest.h>
#include <thread>
#include <iostream>
#include <chrono>
#include <functional>
#include <atomic>


namespace {
void f1(int n) {
  for (size_t idx = 0; idx < 5; ++idx) {
    std::cout << "Thread " << n << ", thread Id is "
              << std::this_thread::get_id() << "executing" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

void f2(int &n) {
  for (size_t idx = 0; idx < 5; ++idx) {
    std::cout << "Thread 2 executing\n";
    ++n;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

struct func;

class scoped_thread {
public:
  explicit scoped_thread(std::thread t_)
      : t(std::move(t_))
  {
    if (!t.joinable())
      throw std::logic_error("No thread");
  }

  ~scoped_thread() {
    t.join();
  }

  scoped_thread(scoped_thread const &) = delete;
  scoped_thread& operator=(scoped_thread const &) = delete;

private:
  std::thread t;
};

struct func {
  int &i;
  explicit func(int &i_) : i(i_) {}

  void operator()() {
    for (auto j = 0; j < 1000000; ++j) {
      // do_something(j);
    }
  }
};


}

TEST(thread_construction, demo) {
    int n = 0;
    std::thread t1; // t1 is not a thread. 
    std::thread t2(f1, n+1); // pass by value.

    std::thread t3(f2, std::ref(n));
    std::thread t4(std::move(t3));  // t4 is now running f2(). t3 is no longer a thread.

    t2.join();
    t4.join();

    std::cout << "Final value of n is " << n << std::endl;
}
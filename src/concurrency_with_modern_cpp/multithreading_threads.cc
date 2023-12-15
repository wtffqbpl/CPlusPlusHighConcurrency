#include <gtest/gtest.h>

#include <thread>
#include <utility>

namespace thread_creation {

void hello_function() {
  std::cout << "Hello from a function." << std::endl;
}

class HelloFunctionObject {
public:
  void operator()() const {
    std::cout << "Hello from a function object." << std::endl;
  }
};

} // namespace thread_creation

TEST(thread_test, creation_test1) {
  std::cout << std::endl;

  std::thread t1(thread_creation::hello_function);

  thread_creation::HelloFunctionObject hello_function_obj{};
  std::thread t2(hello_function_obj);

  std::thread t3([] { std::cout << "Hello from a lambda." << std::endl;});

  t1.join();
  t2.join();
  t3.join();

  std::cout << std::endl;
}

namespace scoped_thread_utils {

class scoped_thread {
private:
  std::thread t;

public:
  explicit scoped_thread(std::thread t_) : t(std::move(t_)) {
    if (!t.joinable())
      throw std::logic_error("No thread");
  }

  ~scoped_thread() { t.join(); }

  scoped_thread(scoped_thread &) = delete;
  scoped_thread& operator=(scoped_thread const &) = delete;
};

}

namespace undefined_behavior_reference_case {

class Sleeper {
public:
  explicit Sleeper(int &i_) : i{i_} {};

  void operator()(int k) {
    for (unsigned int j = 0; j <= 5; ++j) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      i += k;
    }
    std::cout << std::this_thread::get_id() << std::endl;
  }

private:
  int &i;
};

}

TEST(pass_ref_test, test1) {
  std::cout << std::endl;

  using namespace undefined_behavior_reference_case;

  int val_sleeper = 1000;
  std::thread t(Sleeper(val_sleeper), 5);
  t.join();
  // t.detach(); // undefined-behavior

  std::cout << "val_sleeper = " << val_sleeper << std::endl;

  std::cout << "std::thread::hardware_concurrency() = "
            << std::thread::hardware_concurrency()
            << std::endl;

  std::cout << std::endl;
}

namespace native_handle_test {
// std::thread::native_handle
// Returns the implementation defined underlying thread handle.
// May throw implementation-defined exceptions.

// Example: Use native_handle to enable realtime scheduling of C++ threads on a POSIX system

std::mutex iomutex;

void f(int num) {
  std::this_thread::sleep_for(std::chrono::seconds(1));

  sched_param sch{};
  int policy;
  pthread_getschedparam(pthread_self(), &policy, &sch);
  std::lock_guard<std::mutex> lk(iomutex);

  std::cout << "Thread " << num << " is executing at priority "
            << sch.sched_priority << '\n';
}

} // namespace native_handle_test

TEST(native_handle_case, test1) {
  using namespace native_handle_test;

  std::thread t1(f, 1), t2(f, 2);

  sched_param sch{};
  int policy;
  pthread_getschedparam(t1.native_handle(), &policy, &sch);
  sch.sched_priority = 20;

  if (pthread_setschedparam(t1.native_handle(), SCHED_FIFO, &sch))
    std::cout << "Failed to setschedparam: " << std::strerror(errno) << '\n';

  t1.join();
  t2.join();
}
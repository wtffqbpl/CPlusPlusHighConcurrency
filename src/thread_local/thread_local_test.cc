#include <gtest/gtest.h>
#include <mutex>
#include <thread>
#include <iostream>

namespace {
class A {
public:
  A() = default;
  ~A() = default;

  void test(const std::string &name) {
    thread_local int count = 0;
    ++count;
    std::cout << name << ": " << count << std::endl;
    std::flush(std::cout);
  }
};

void func(const std::string &name) {
  A a1;
  a1.test(name);
  a1.test(name);

  A a2;
  a2.test(name);
  a2.test(name);
}

void thread_local_test1() {
  std::thread t1(func, "t1");
  t1.join();
  std::thread t2(func, "t2");
  t2.join();
}

}

TEST(thread_local_test, test1) {
  std::stringstream oss;
  testing::internal::CaptureStdout();

  thread_local_test1();

  auto act_output = testing::internal::GetCapturedStdout();
  oss << "t1: 1\n"
         "t1: 2\n"
         "t1: 3\n"
         "t1: 4\n"
         "t2: 1\n"
         "t2: 2\n"
         "t2: 3\n"
         "t2: 4\n";

#ifndef NDEBUG
  std::cout << "Expected output:\n" << oss.str() << '\n'
            << "Actual output:\n" << act_output << '\n';
#endif

  EXPECT_TRUE(oss.str() == act_output);
}
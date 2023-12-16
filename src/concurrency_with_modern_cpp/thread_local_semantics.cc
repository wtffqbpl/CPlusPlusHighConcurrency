#include <gtest/gtest.h>
#include <string>
#include <mutex>
#include <thread>

namespace thread_local_cases {

std::mutex cout_mutex;

thread_local std::string s("hello from ");

void add_thread_local(std::string const &s2) {
  s += s2;

  // protect std::cout
  std::lock_guard<std::mutex> guard(cout_mutex);
  std::cout << "s: " << s << ", ";
  std::cout << "&s: " << &s << std::endl;
}

} // namespace thread_local_cases

TEST(concurrency_book_thread_local_test, test1) {
  using namespace thread_local_cases;

  std::thread t1(add_thread_local, "t1");
  std::thread t2(add_thread_local, "t2");
  std::thread t3(add_thread_local, "t3");
  std::thread t4(add_thread_local, "t4");

  t1.join();
  t2.join();
  t3.join();
  t4.join();
}
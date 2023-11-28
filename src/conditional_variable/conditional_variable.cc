#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <iterator>
#include <thread>
#include <vector>

namespace conditional_variable_test {

std::vector<int> my_shared_work;
std::mutex mutex_;
std::condition_variable cond_var;

bool data_ready{false};

void waiting_for_work() {
  std::cout << "Waiting " << std::endl;
  std::unique_lock<std::mutex> lck{mutex_};
  cond_var.wait(lck, [] { return data_ready; });
  my_shared_work[1] = 2;
  std::cout << "work done " << std::endl;
}

void set_data_ready() {
  my_shared_work = {1, 0, 3};
  {
    std::lock_guard<std::mutex> lck{mutex_};
    data_ready = true;
  }

  std::cout << "Data prepared" << std::endl;
  cond_var.notify_one();
}

} // namespace conditional_variable_test

TEST(condition_variable_test, test1)
{
  std::cout << '\n';

  std::thread t1(conditional_variable_test::waiting_for_work);
  std::thread t2(conditional_variable_test::set_data_ready);

  t1.join();
  t2.join();

  for (auto v : conditional_variable_test::my_shared_work)
    std::cout << v << ' ';

  std::cout << '\n';
}

namespace atomic_test {

std::vector<int> my_shared_work;
std::atomic<bool> data_ready{false};

void waiting_for_work() {
  std::cout << "Waiting " << std::endl;

  while (data_ready.load()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }

  my_shared_work[1] = 2;
  std::cout << "Work done " << std::endl;
}

void set_data_ready() {
  my_shared_work = {1, 0, 3};
  data_ready = true;
  std::cout << "Data prepared" << std::endl;
}

} // namespace atomic_test

TEST(atomic_test, test1) {
  std::cout << std::endl;

  std::thread t1(atomic_test::waiting_for_work);
  std::thread t2(atomic_test::set_data_ready);

  t1.join();
  t2.join();

  std::copy(atomic_test::my_shared_work.begin(), atomic_test::my_shared_work.end(),
            std::ostream_iterator<int>(std::cout, " "));
  std::cout << std::endl;
}
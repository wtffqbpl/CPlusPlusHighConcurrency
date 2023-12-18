#include <gtest/gtest.h>
#include <chrono>
#include <future>
#include <thread>
#include <numeric>
#include <random>

namespace task_utils {

// Tasks versus threads
// |-----------------------------------------------------------------------------------------------------------
// |        Criteria           |        Threads                       |           Tasks                       |
// |---------------------------+--------------------------------------+----------------------------------------
// | Participants              | creator and child thread             | promise and future                    |
// |---------------------------+--------------------------------------+----------------------------------------
// | Communication             | shared variable                      | communication channel                 |
// |---------------------------+--------------------------------------+----------------------------------------
// | Thread creation           | obligatory                           | optional                              |
// |---------------------------+--------------------------------------+----------------------------------------
// | Synchronisation           | via join() (waits)                   | get call blocks                       |
// |---------------------------+--------------------------------------+----------------------------------------
// | Exception in child thread | child and creator threads terminates | rturn value of the promise            |
// |---------------------------+--------------------------------------+----------------------------------------
// | Kins of communication     | values                               | values, notifications, and exceptions |
// |-----------------------------------------------------------------------------------------------------------
//
// Threads need the <thread> header; tasks need the <future> header.

void task_basic_test() {
  std::cout << std::endl;

  int res;
  std::thread t([&] { res = 2000 + 11; });
  t.join();
  std::cout << "res: " << res << std::endl;

  auto fut = std::async([] { return 2000 + 11; });
  std::cout << "fut.get(): " << fut.get() << std::endl;

  std::cout << std::endl;
}

// The `std::launch` enumeration has three possible values:
// 1. std::launch::async: The function is executed asynchronously in a separate
//    thread. The `std::future` returned by `std::async` will eventually hold
//    the result of the asynchronous operation.
// 2. std::launch::deferred: The function is executed synchronously when the
//    `std::future` obtained from `std::async` is queried. In other words, the
//    function is not executed until the `get` member function is called on the
//    `std::future` object.
// 3. std::launch::async | std::launch::deferred: The implementation is allowed
//    to choose whether to execute the function asynchronously or deferred.
// The default launch policy is `std::launch::async | std::launch::deferred`.
// This means that the implementation can devide whether to execute the fucntion
// asynchronously or synchronously based on factors such as the availability of
// resources or the workload of the system.

void task_async_launch_policy_test() {
  // Using std::launch::async
  auto async_future = std::async(std::launch::async, [] {
    std::cout << "Async task running in a separate thread.\n";
    return 42;
  });

  // Using std::launch::deferred
  auto deferred_future = std::async(std::launch::deferred, [] {
    std::cout << "deferred task running synchronously.\n";
    return 84;
  });

  // Wait for the asynchronous task to complete and retrieve the result.
  int async_result = async_future.get();
  std::cout << "Async result: " << async_result << std::endl;

  // Wait for the deferred task to complete and retrieve the result
  int deferred_result = deferred_future.get();
  std::cout << "Deferred result: " << deferred_result << std::endl;
}

void test_async_launch_policy_test2() {
  auto begin = std::chrono::system_clock::now();

  auto async_lazy = std::async(std::launch::deferred,
                               [] { return std::chrono::system_clock::now(); });
  auto async_eager = std::async(std::launch::async,
                                [] { return std::chrono::system_clock::now(); });

  std::this_thread::sleep_for(std::chrono::seconds(1));

  auto lazy_start = async_lazy.get() - begin;
  auto eager_start = async_eager.get() - begin;

  auto lazy_duration = std::chrono::duration<double>(lazy_start).count();
  auto eager_duration = std::chrono::duration<double>(eager_start).count();

  std::cout << "async_lazy evaluated after: " << lazy_duration << " seconds." << std::endl;
  std::cout << "async_eager evaluated after: " << eager_duration << " seconds." << std::endl;

  std::cout << std::endl;
}

// Fire and forget futures
// --- which is the std::async with std::launch::async launch policy.

void fire_and_forget_future_test() {
  (void )std::async(std::launch::async, [] {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "first thread." << std::endl;
  });

  (void)std::async(std::launch::async, [] {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "second thread" << std::endl;
  });

  std::cout << "main thread" << std::endl;
  std::cout << std::endl;
}

// Example: concurrent calculation

static constexpr int NUM = 100000000;

long long get_dot_product(std::vector<int> &v, std::vector<int> &w) {
  auto size = v.size();

  auto future1 = std::async([&] {
    return std::inner_product(&v[0], &v[size / 4], &w[0], 0ll);
  });

  auto future2 = std::async([&] {
    return std::inner_product(&v[size / 4], &v[size / 2], &w[size / 4], 0ll);
  });

  auto future3 = std::async([&] {
    return std::inner_product(&v[size / 2], &v[size * 3 / 4], &w[size / 2], 0ll);
  });

  auto future4 = std::async([&] {
    return std::inner_product(&v[size * 3 / 4], &v[size], &w[size * 3 / 4], 0ll);
  });

  return future1.get() + future2.get() + future3.get() + future4.get();
}

} // namespace task_utils

TEST(task_test, basic_test) {
  using namespace task_utils;

  task_basic_test();
}

TEST(task_test, launch_policy_test1) {
  using namespace task_utils;

  task_async_launch_policy_test();
}

TEST(task_test, launch_policy_test2) {
  using namespace task_utils;

  test_async_launch_policy_test2();
}

TEST(task_test, fire_and_forget_future_test) {
  using namespace task_utils;

  fire_and_forget_future_test();
}

TEST(task_test, concurrent_calculation_example) {
  using namespace task_utils;

  std::random_device seed;

  // generator
  std::mt19937 engine(seed());

  // distribution
  std::uniform_int_distribution<int> dist(0, 100);

  // fill the vectors
  std::vector<int> v, w;
  v.reserve(NUM);
  w.reserve(NUM);

  for (int i = 0; i < NUM; ++i) {
    v.push_back(dist(engine));
    w.push_back(dist(engine));
  }

  std::cout << "get_dot_product(v, w) = " << get_dot_product(v, w) << std::endl;
}
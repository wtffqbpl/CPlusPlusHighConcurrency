#include <gtest/gtest.h>
#include <chrono>
#include <future>
#include <thread>
#include <numeric>
#include <random>
#include <utility>
#include <deque>

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

namespace packaged_task_utils {

// Here's a brief explanation of how `std::packaged_task` works:

// 1. Wrap your work:
/// \code
/// std::packaged_task<int(int, int)> sum_task([] (int a, int b) { return a + b; });
/// \endcode
//
// 2.Create a future:
/// \code
/// std::future<int> sum_result = sum_task.get_future();
/// \endcode
//
// 3. Perform the calculation:
///\code
/// sum_task(2000, 11);
///\endcode
//
// 4. Query the result:
/// \code
/// sum_result.get();
/// \endcode

// `std::packaged_task` is useful for scenarios where you want to decouple the
// definition of a task from its execution and retrieval of results. It's often
// used in conjunction with asynchronous programming, where you can execute
// tasks concurrently and collect their results when needed.
void basic_example() {
  // Step 1: wrap your work:
  std::packaged_task<int()> task([]() { return 42; });

  // Step 2: Create a future:
  std::future<int> res_fut = task.get_future();

  // Step 3: Perform the calculation:
  // Synchronous execution
  task(); // The program waits for the task to complete

  // Step 4: Query the result:
  int result = res_fut.get();

  std::cout << "result = " << result << std::endl;
}

class SumUp {
public:
  int operator()(int beg, int end) const {
    long long int sum{0};
    for (int i = beg; i < end; ++i) sum += i;
    return static_cast<int>(sum);
  }
};

void packaged_task_example() {
  SumUp sum_up1{};
  SumUp sum_up2{};
  SumUp sum_up3{};
  SumUp sum_up4{};

  // wrap the tasks
  std::packaged_task<int(int, int)> sum_task1(sum_up1);
  std::packaged_task<int(int, int)> sum_task2(sum_up2);
  std::packaged_task<int(int, int)> sum_task3(sum_up3);
  std::packaged_task<int(int, int)> sum_task4(sum_up4);

  // Create the futures
  std::future<int> sum_result1 = sum_task1.get_future();
  std::future<int> sum_result2 = sum_task2.get_future();
  std::future<int> sum_result3 = sum_task3.get_future();
  std::future<int> sum_result4 = sum_task4.get_future();

  // Push the tasks on the container
  std::deque<std::packaged_task<int(int, int)>> all_tasks;
  all_tasks.push_back(std::move(sum_task1));
  all_tasks.push_back(std::move(sum_task2));
  all_tasks.push_back(std::move(sum_task3));
  all_tasks.push_back(std::move(sum_task4));

  int begin{1};
  int increment{2500};
  int end = begin + increment;

  // Perform each calculation in a separate thread
  while (not all_tasks.empty()) {
    std::packaged_task<int(int, int)> my_task = std::move(all_tasks.front());
    all_tasks.pop_front();

    std::thread sum_thread(std::move(my_task), begin, end);
    begin = end;
    end += increment;
    sum_thread.detach();
  }

  // pick up the results
  auto sum = sum_result1.get() + sum_result2.get() + sum_result3.get() + sum_result4.get();

  std::cout << "sum of 0 ... 10000 = " << sum << std::endl;

  std::cout << std::endl;
}

// Using `std::packaged_task` to parallel the computation of Fibonacci numbers.

// Function to calculate Fibonacci numbers.
unsigned long long fibonacci(unsigned int n) {
  if (n <= 1) return n;

  return fibonacci(n - 1) + fibonacci(n - 2);
}

void fibonacci_calc_with_packaged_task() {
  constexpr unsigned int n = 40; // Calculate Fibonacci numbers up to n;

  std::vector<std::packaged_task<unsigned long long()>> tasks;
  std::vector<std::future<unsigned long long>> futures;

  // Create tasks and associated futures.
  for (unsigned int i = 0; i <= n; ++i) {
    tasks.emplace_back([i]() {
      return fibonacci(i);
    });
    futures.emplace_back(tasks.back().get_future());
  }

  // Execute tasks asynchronously
  for (auto &task : tasks) {
    // Detach threads for asynchronous execution.
    std::thread(std::move(task)).detach();
  }

  // Wait for the results and accumulate the sum
  unsigned long long sum = 0;
  for (auto &future : futures)
    sum += future.get();

  // Print the results.
  std::cout << "Sum of fibonacci numbers up to " << n << ": " << sum << std::endl;
}

// A std::packaged_task can be in contrast to a std::async, or a std::promise reset
// and reused. The following example shows this special use-case on a std::packaged_task.

void calc_products(std::packaged_task<int(int, int)> &task,
                   const std::vector<std::pair<int, int>> &pairs) {
  for (auto &pair : pairs) {
    auto fut = task.get_future();
    task(pair.first, pair.second);
    std::cout << pair.first << " * " << pair.second << " = " << fut.get() << std::endl;
    // Resets the state of the task. Abandons the stored results from previous execution.
    task.reset();
  }
}

void packaged_task_reset_example() {
  std::vector<std::pair<int, int>> all_pairs;
  all_pairs.emplace_back(1, 2);
  all_pairs.emplace_back(2, 3);
  all_pairs.emplace_back(3, 4);
  all_pairs.emplace_back(4, 5);

  std::packaged_task<int(int, int)> task{[](int first, int second) {
    return first * second; }};

  calc_products(task, all_pairs);

  std::cout << std::endl;

  std::thread t(calc_products, std::ref(task), all_pairs);
  t.join();
}


} // namespace packaged_task_utils

TEST(packaged_task_test, basic_example) {
  using namespace packaged_task_utils;

  basic_example();
}

TEST(packaged_task_test, example1) {
  using namespace packaged_task_utils;

  packaged_task_example();
}

TEST(packaged_task_test, example2) {
  using namespace packaged_task_utils;

  fibonacci_calc_with_packaged_task();
}

TEST(packaged_task_test, reset_example) {
  using namespace packaged_task_utils;

  packaged_task_reset_example();
}

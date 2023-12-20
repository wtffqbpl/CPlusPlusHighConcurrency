#include <gtest/gtest.h>
#include <algorithm>
#include <vector>
#include <chrono>
#include <future>
#include <random>
#include <utility>

#ifdef PARALLEL
#include <execution>
  namespace execution = std::execution;
#else
  enum class execution { seq, unseq, par_unseq, par };
#endif

namespace stl_par_seq_test {

// std::execution::seq  runs the program sequentially
// std::execution::par  runs the program in parallel on multiple threads
// std::execution::par_unseq  runs the program in parallel on multiple threads
//                      and allows the interleaving of individual loops; permits
//                      a vectorized version with SIMD extensions.

void test_par_sort() {
  std::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9};

  // standard sequential sort
  std::sort(v.begin(), v.end());
}


} // namespace stl_par_seq_test

TEST(parallel_algorithms_test, test) {
  using namespace stl_par_seq_test;

  test_par_sort();
}

namespace {
constexpr long long size = 100000000;

constexpr long long fir = 25000000;
constexpr long long sec = 50000000;
constexpr long long thi = 75000000;
constexpr long long fou = 100000000;

void sum_up(std::promise<unsigned long long> &&prom,
            const std::vector<int> &val,
            unsigned long long beg,
            unsigned long long end) {
  unsigned  long long sum{};

  for (auto i =beg; i < end; ++i)
    sum += val[i];
  prom.set_value(sum);
}

void test_performance() {
  std::vector<int> rand_values;
  rand_values.reserve(size);

  std::mt19937  g;
  std::uniform_int_distribution<> uniform_dist(1, 10);

  for (long long i = 0; i < size; ++i)
    rand_values.push_back(uniform_dist(g));

  std::promise<unsigned long long> prom1;
  std::promise<unsigned long long> prom2;
  std::promise<unsigned long long> prom3;
  std::promise<unsigned long long> prom4;

  auto fut1 = prom1.get_future();
  auto fut2 = prom2.get_future();
  auto fut3 = prom3.get_future();
  auto fut4 = prom4.get_future();

  const auto sta = std::chrono::system_clock::now();

  std::thread t1(sum_up, std::move(prom1), std::ref(rand_values), 0, fir);
  std::thread t2(sum_up, std::move(prom2), std::ref(rand_values), fir, sec);
  std::thread t3(sum_up, std::move(prom3), std::ref(rand_values), sec, thi);
  std::thread t4(sum_up, std::move(prom4), std::ref(rand_values), thi, fou);

  auto sum = fut1.get() + fut2.get() + fut3.get() + fut4.get();

  std::chrono::duration<double> dur = std::chrono::system_clock::now() - sta;

  std::cout << "Time for addition " << dur.count()
            << " seconds." << std::endl;
  std::cout << "Result: " << sum << std::endl;

  t1.join();
  t2.join();
  t3.join();
  t4.join();
}

}

TEST(performance_test, test) {
  test_performance();
}
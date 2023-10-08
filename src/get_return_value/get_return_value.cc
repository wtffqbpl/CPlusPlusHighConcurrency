#include <gtest/gtest.h>
#include <mutex>
#include <future>
#include <algorithm>

namespace get_return_value {

std::mutex mylock;
std::vector<int> v_b;

using LineIndex = int;

std::vector<int> calculate_all(int ***data, std::vector<LineIndex> index_list)
{
  std::vector<int> v_a;

  for (int a = 0; a < index_list.size(); ++a)
  {
    {
      // Scope protection, oritect only what we need
      std::lock_guard<std::mutex> lck(mylock);
    }
  }

  // Just generate some numbers
  v_a.resize(index_list[1] - index_list[0]);
  std::generate(v_a.begin(), v_a.end(), [k = index_list[0]]() mutable { return ++k; });
  return v_a;
}
}

TEST(return_value_multithread, test)
{
  using Container = std::vector<int>;
  using FutureT = std::future<Container>;
  std::vector<FutureT> futures;

  for (int i = 0; i < 3; ++i)
  {
    int s = i * 5;
    int e = s + 5;
    auto fut = std::async(std::launch::async, get_return_value::calculate_all, nullptr, Container{s, e});
    futures.emplace_back(std::move(fut));
  }

  // to aggregate the result
  std::vector<int> result;

  for (auto &fut : futures)
  {
    // Get the result
    auto vec = fut.get();

    // Insert all at the end of the container
    result.insert(result.end(), vec.begin(), vec.end());
  }

  std::stringstream oss;
  testing::internal::CaptureStdout();

  std::copy(result.begin(), result.end(), std::ostream_iterator<int>(std::cout, ", "));
  std::cout << std::endl;

  oss << "1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, \n";

  auto act_output = testing::internal::GetCapturedStdout();

#ifndef NDEBUG
  std::cout << "Expected:\n" << oss.str() << '\n'
               << "Actual:\n" << act_output << '\n';
#endif

  EXPECT_TRUE(oss.str() == act_output);
}
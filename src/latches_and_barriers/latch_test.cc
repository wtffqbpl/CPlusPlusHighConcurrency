#include <gtest/gtest.h>
#include <functional>
#include <iostream>
#include <latch>
#include <string>
#include <thread>

namespace {

struct Job {
  const std::string name;
  std::string product{"hot worked"};
  std::thread action{};
};

void latch_test()
{
  Job jobs[]{{"Annika"}, {"Buru"}, {"Chuck"}};

  std::latch work_done{std::size(jobs)};
  std::latch start_clean_up{1};

  auto work = [&](Job &my_job) {
    my_job.product = my_job.name + " worked";
    work_done.count_down();
    start_clean_up.wait();
    my_job.product = my_job.name + " cleaned";
  };

  std::cout << "Work is starting...";
  for (auto &job : jobs)
    job.action = std::thread{work, std::ref(job)};

  work_done.wait();
  std::cout << "done:\n";
  for (auto const &job : jobs)
    std::cout << " " << job.product << '\n';

  std::cout << "Workers are cleaning up... ";
  start_clean_up.count_down();
  for (auto &job : jobs)
    job.action.join();

  std::cout << "done:\n";
  for (auto const &job : jobs)
    std::cout << " " << job.product << '\n';
}

}

TEST(latch_test, test1)
{
  latch_test();
}
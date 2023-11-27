#include <gtest/gtest.h>
#include <atomic>
#include <thread>

namespace {
class Spinlock {
  std::atomic_flag flag = ATOMIC_FLAG_INIT;

public:
  void lock() { while (flag.test_and_set()); }

  void unlock() { flag.clear(); }
};

Spinlock spin;

void work_on_resource() {
  spin.lock();
  // shared resource
  std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  std::cout << "thread_id = " << std::this_thread::get_id() << std::endl;
  spin.unlock();
}

}

TEST(spinlock_with_atomic_flag, test1)
{
  std::thread t1(work_on_resource);
  std::thread t2(work_on_resource);

  t1.join();
  t2.join();
}
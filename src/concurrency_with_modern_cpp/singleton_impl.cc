#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <future>
#include <mutex>
#include <thread>

namespace case_studies {

std::mutex mtx;

class Singleton {
public:
  static Singleton &get_instance() {
    if (!instance) {
      std::lock_guard<std::mutex> lock(mtx);
      if (!instance) {
        instance = new Singleton{};
      }
    }
    return *instance;
  }

  Singleton(const Singleton &) = delete;
  Singleton operator=(const Singleton &) = delete;

private:
  Singleton() = default;
  ~Singleton() = default;

private:
  static Singleton *instance;
};

constexpr auto ten_mill = 10000000;

class SingletonThreadSafe {
public:

  static SingletonThreadSafe &get_instance() {
    static SingletonThreadSafe instance;
    return instance;
  }

  SingletonThreadSafe(const SingletonThreadSafe &) = delete;
  SingletonThreadSafe operator=(const SingletonThreadSafe &) = delete;

private:
  SingletonThreadSafe() = default;
  ~SingletonThreadSafe() = default;
};

std::chrono::duration<double> get_time() {
  auto begin = std::chrono::system_clock::now();
  for (size_t i = 0; i < ten_mill; ++i) {
    SingletonThreadSafe::get_instance();
  }

  return std::chrono::system_clock::now() - begin;
}

void test_singleton_performance() {
  auto fut1 = std::async(std::launch::async, get_time);
  auto fut2 = std::async(std::launch::async, get_time);
  auto fut3 = std::async(std::launch::async, get_time);
  auto fut4 = std::async(std::launch::async, get_time);

  const auto total = fut1.get() + fut2.get() + fut3.get() + fut4.get();

  std::cout << total.count() << std::endl;
}

class SingletonCallOnce {
public:
  static SingletonCallOnce &get_instance() {
    std::call_once(init_instance_flag, &SingletonCallOnce::init_singleton);
    return *instance;
  }

  SingletonCallOnce(const SingletonCallOnce &) = delete;
  SingletonCallOnce &operator=(const SingletonCallOnce &) = delete;

private:
  SingletonCallOnce() = default;
  ~SingletonCallOnce() = default;

  static void init_singleton() {
    instance = new SingletonCallOnce{};
  }

private:
  static SingletonCallOnce *instance;
  static std::once_flag init_instance_flag;
};

SingletonCallOnce *SingletonCallOnce::instance = nullptr;
std::once_flag SingletonCallOnce::init_instance_flag;

std::chrono::duration<double> get_call_once_singleton_time() {
  auto begin = std::chrono::system_clock::now();

  for (size_t i = 0; i < ten_mill; ++i) {
    SingletonCallOnce::get_instance();
  }

  return std::chrono::system_clock::now() - begin;
}

void test_call_once_singleton_performance() {
  auto fut1 = std::async(std::launch::async, get_call_once_singleton_time);
  auto fut2 = std::async(std::launch::async, get_call_once_singleton_time);
  auto fut3 = std::async(std::launch::async, get_call_once_singleton_time);
  auto fut4 = std::async(std::launch::async, get_call_once_singleton_time);

  const auto total = fut1.get() + fut2.get() + fut3.get() + fut4.get();

  std::cout << total.count() << std::endl;
}

// Acquire-Release semantics

class SingletonAcquireRelease {
public:
  static SingletonAcquireRelease &get_instance() {
    auto *sin = instance.load(std::memory_order_acquire);
    if (!sin) {
      std::lock_guard<std::mutex> lock(mtx);
      sin = instance.load(std::memory_order_acquire);
      if (!sin) {
        sin = new SingletonAcquireRelease{};
        instance.store(sin, std::memory_order_release);
      }
    }
    return *sin;
  }

  SingletonAcquireRelease(const SingletonAcquireRelease &) = delete;
  SingletonAcquireRelease &operator=(const SingletonAcquireRelease &) = delete;

private:
  SingletonAcquireRelease() = default;
  ~SingletonAcquireRelease() = default;

private:
  static std::atomic<SingletonAcquireRelease*> instance;
  static std::mutex mtx;

};

std::atomic<SingletonAcquireRelease *> SingletonAcquireRelease::instance;
std::mutex SingletonAcquireRelease::mtx;

std::chrono::duration<double> get_require_release_singleton_time() {
  auto begin = std::chrono::system_clock::now();

  for (size_t i = 0; i < ten_mill; ++i) {
    SingletonAcquireRelease::get_instance();
  }

  return std::chrono::system_clock::now() - begin;
}

void test_acquire_release_singleton_performance() {
  auto fut1 = std::async(std::launch::async, get_require_release_singleton_time);
  auto fut2 = std::async(std::launch::async, get_require_release_singleton_time);
  auto fut3 = std::async(std::launch::async, get_require_release_singleton_time);
  auto fut4 = std::async(std::launch::async, get_require_release_singleton_time);

  const auto total = fut1.get() + fut2.get() + fut3.get() + fut4.get();
  std::cout << total.count() << std::endl;
}

} // namespace case_studies

TEST(case_studies_test, test1) {
  using namespace case_studies;

  test_singleton_performance();
}

TEST(case_studies_test, test2) {
  using namespace case_studies;

  test_call_once_singleton_performance();
}

TEST(case_studies_test, test3) {
  using namespace case_studies;

  test_acquire_release_singleton_performance();
}

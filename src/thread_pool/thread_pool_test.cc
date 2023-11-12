#include <gtest/gtest.h>
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace {

template <typename T>
class ThreadSafeQueue {
private:
  mutable std::mutex mut;
  std::queue<T> data_queue;
  std::condition_variable data_cond;

public:
  void push(T new_value) {
    std::lock_guard<std::mutex> lk(mut);
    data_queue.push(new_value);
    data_cond.notify_one();
  }

  void wait_and_pop(T &value) {
    std::unique_lock<std::mutex> lk(mut);
    data_cond.wait(lk, [this] { return !data_queue.empty(); });
    value = data_queue.front();
    data_queue.pop();
  }

  std::shared_ptr<T> wait_and_pop() {
    std::unique_lock<std::mutex> lk(mut);
    data_cond.wait(lk, [this] { return !data_queue.empty(); });
    std::shared_ptr<T> res(
        std::make_shared<T>(std::move(data_queue.front())));
    data_queue.pop();
    return res;
  }

  bool try_pop(T &value) {
    std::lock_guard<std::mutex> lk(mut);
    if (data_queue.empty())
      return false;

    value = std::move(data_queue.front());
    data_queue.pop();
    return true;
  }

  std::shared_ptr<T> try_pop() {
    std::lock_guard<std::mutex> lk(mut);
    if (data_queue.empty())
      return std::shared_ptr<T>();

    std::shared_ptr<T> res(
        std::make_shared<T>(std::move(data_queue.front())));
    data_queue.pop();

    return res;
  }

  bool empty() const {
    std::lock_guard<std::mutex> lk(mut);
    return data_queue.empty();
  }
};

class JoinThreads {
  std::vector<std::thread> &threads;

public:
  explicit JoinThreads(std::vector<std::thread> &threads_)
    : threads(threads_)
  {}

  ~JoinThreads() {
    for (auto & thread : threads) {
      if (thread.joinable())
        thread.join();
    }
  }
};

class ThreadPool {
  std::atomic_bool done;
  ThreadSafeQueue<std::function<void()>> work_queue;
  std::vector<std::thread> threads;
  JoinThreads joiner;

  void worker_thread() {
    while (!done) {
      std::function<void()> task;
      if (work_queue.try_pop(task)) {
        task();
      } else {
        std::this_thread::yield();
      }
    }
  }

public:
  ThreadPool()
      : done(false), joiner(threads)
  {
    unsigned const thread_count = std::thread::hardware_concurrency();

    try {
      for (unsigned i = 0; i < thread_count; ++i) {
        threads.emplace_back(&ThreadPool::worker_thread, this);
      }
    }
    catch (...) {
      done = true;
      throw;
    }
  }

  ~ThreadPool() { done = true; }

  template <typename FunctionType>
  void submit(FunctionType f) {
    work_queue.push(std::function<void()>(f));
  }
};

}
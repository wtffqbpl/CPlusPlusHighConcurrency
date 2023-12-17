#include <gtest/gtest.h>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <queue>

namespace conditional_variable_utils {
std::mutex mutex_;
std::condition_variable cond_var;

bool data_ready{false};

void do_the_work() {
  std::cout << "Processing shared data." << std::endl;
}

void waiting_for_work() {
  std::cout << "Worker: Waiting for work." << std::endl;

  std::unique_lock<std::mutex> lck(mutex_);
  cond_var.wait(lck, [] { return data_ready; });
  data_ready = false;
  do_the_work();
  std::cout << "Work done." << std::endl;
}

void set_data_ready() {
  {
    std::lock_guard<std::mutex> lck(mutex_);
    data_ready = true;
  }

  std::cout << "Sender: Data is ready." << std::endl;
  cond_var.notify_one();
}

} // namespace conditional_variable_utils

TEST(condition_varaibles_test, test1) {
  using namespace conditional_variable_utils;

  std::thread t1(waiting_for_work);
  std::thread t2(set_data_ready);
  std::thread t3(waiting_for_work);

  t1.join();
  t2.join();
  t3.join();

  std::cout << std::endl;
}

namespace condition_variable_consumer_producer {

constexpr int BUFFER_SIZE = 10;
std::queue<int> buffer;
std::mutex mtx;
std::condition_variable buffer_not_empty;
std::condition_variable buffer_not_full;

void producer(int id) {
  for (int i = 0; i < 20; ++i) {
    std::unique_lock<std::mutex> lock(mtx);

    buffer_not_full.wait(lock, []() { return buffer.size() < BUFFER_SIZE; });

    // Produce an item and add it to the buffer.
    int item = i + id * 1000;
    buffer.push(item);
    std::cout << "Producer " << id << " produced item: " << item << std::endl;

    lock.unlock();
    buffer_not_empty.notify_one();
  }
}

void consumer(int id) {

  for (int i = 0; i < 20; ++i) {
    std::unique_lock<std::mutex> lock(mtx);
    buffer_not_empty.wait(lock, []() { return !buffer.empty(); });

    // Consume an item from the buffer
    int item = buffer.front();
    buffer.pop();

    std::cout << "Consumer " << id << " consumed item: " << item << std::endl;

    lock.unlock();
    buffer_not_full.notify_one();
  }
}

class Buffer {
public:
  explicit Buffer(int size) : data(size) ,head(0), tail(0) {}

  void produce(int item) {
    std::unique_lock<std::mutex> lock(mtx);

    while ((tail + 1) % data.size() == head) { // Buffer full
      cv.wait(lock);
    }

    data[tail] = item;
    tail = (tail + 1) % data.size();
    cv.notify_one(); // Notify consumer of available item
  }

  int consume() {
    std::unique_lock<std::mutex> lock(mtx);
    while (head == tail) // Buffer empty
      cv.wait(lock);

    int item = data[head];
    head = (head + 1) % data.size();

    cv.notify_one(); // Notify producer of space available
    return item;
  }

private:
  std::vector<int> data;
  int head, tail;
  std::mutex mtx;
  std::condition_variable cv;
};

void print_odd_even_numbers_in_different_threads() {
  bool ready = true;

  int i = 0;
  int n = 100;
  std::mutex mtx;
  std::condition_variable cv;

  // t1 print odd number
  std::thread t1([&]() {
    while (i < n) {
      std::unique_lock<std::mutex> lock(mtx);
      // when ready is false, then weak up thread1
      cv.wait(lock, [&ready]() { return !ready; });

      std::cout << "t1: " << std::this_thread::get_id() <<": " << i << std::endl;
      ++i;

      ready = true;
      cv.notify_one();
    }
  });

  // t2 prints even number
  std::thread t2([&]() {
    while (i < n) {
      std::unique_lock<std::mutex> lock(mtx);

      // When ready is true, weak up thread2
      cv.wait(lock, [&ready]() { return ready; });

      std::cout << "t2: " << std::this_thread::get_id() << ": " << i << std::endl;
      ++i;
      ready = false;

      cv.notify_one();
    }
  });

  std::this_thread::sleep_for(std::chrono::seconds(3));

  std::cout << "t1: " << t1.get_id() << std::endl;
  std::cout << "t2: " << t2.get_id() << std::endl;

  t1.join();
  t2.join();
}

} // namespace condition_variable_consumer_producer

TEST(condition_variable_test, producer_consumer_test) {
  using namespace condition_variable_consumer_producer;

#if 0 // FIXME: This has some deadlock issues.
  std::vector<std::thread> producers;
  std::vector<std::thread> consumers;

  // Create three producer threads
  for (int i = 0; i < 3; ++i)
    producers.emplace_back(producer, i);

  // Create two consumer threads
  for (int i = 0; i < 2; ++i)
    consumers.emplace_back(consumer, i);

  // Join all threads
  for (auto &producer_thread : producers)
    producer_thread.join();

  for (auto &consumer_thread : consumers)
    consumer_thread.join();
#endif
}

TEST(condition_variable_test, producer_consumer_test2) {
  using namespace condition_variable_consumer_producer;

  Buffer buffer_loc(5);

  std::thread producer([&buffer_loc]() {
    for (int i = 0; i < 10; ++i) {
      buffer_loc.produce(i);
      std::cout << "Produced item: " << i << std::endl;
    }
  });

  std::thread consumer([&buffer_loc]() {
    while (true) {
      int item = buffer_loc.consume();
      std::cout << "Consumed item: " << item << std::endl;
      if (item == 9) break;
    }
  });

  producer.join();
  consumer.join();
}

TEST(condition_variable_test, print_odd_even_test) {
  using namespace condition_variable_consumer_producer;

  print_odd_even_numbers_in_different_threads();
}


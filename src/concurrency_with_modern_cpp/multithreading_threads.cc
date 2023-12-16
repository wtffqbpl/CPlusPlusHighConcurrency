#include <gtest/gtest.h>

#include <mutex>
#include <shared_mutex>
#include <thread>
#include <utility>

namespace thread_creation {

void hello_function() {
  std::cout << "Hello from a function." << std::endl;
}

class HelloFunctionObject {
public:
  void operator()() const {
    std::cout << "Hello from a function object." << std::endl;
  }
};

void test() {
  std::cout << std::boolalpha << std::endl;

  std::cout << "std::hardware_concurrency() = " << std::thread::hardware_concurrency() << std::endl;

  std::thread t1([] { std::cout << "t1 with id = " << std::this_thread::get_id() << std::endl; });
  std::thread t2([] { std::cout << "t2 with id = " << std::this_thread::get_id() << std::endl; });

  std::cout << std::endl;

  std::cout << "FROM MAIN: id of t1 " << t1.get_id() << std::endl;
  std::cout << "FROM MAIN: id of t2 " << t2.get_id() << std::endl;

  std::cout << std::endl;
  std::swap(t1, t2);

  std::cout << "FROM MAIN: id of t1 " << t1.get_id() << std::endl;
  std::cout << "FROM MAIN: id of t2 " << t2.get_id() << std::endl;

  std::cout << std::endl;

  std::cout << "FROM MAIN: id of main = " << std::this_thread::get_id() << std::endl;

  std::cout << std::endl;

  std::cout << "t1.joinable(): " << t1.joinable() << std::endl;

  std::cout << std::endl;

  t1.join();
  t2.join();

  std::cout << std::endl;

  std::cout << "t1.joinable(): " << t1.joinable() << std::endl;

  std::cout << std::endl;
}

} // namespace thread_creation

TEST(thread_test, creation_test1) {
  std::cout << std::endl;

  std::thread t1(thread_creation::hello_function);

  thread_creation::HelloFunctionObject hello_function_obj{};
  std::thread t2(hello_function_obj);

  std::thread t3([] { std::cout << "Hello from a lambda." << std::endl;});

  t1.join();
  t2.join();
  t3.join();

  std::cout << std::endl;
}

TEST(thread_test, api_test1) {
  thread_creation::test();
}

namespace scoped_thread_utils {

class scoped_thread {
private:
  std::thread t;

public:
  explicit scoped_thread(std::thread t_) : t(std::move(t_)) {
    if (!t.joinable())
      throw std::logic_error("No thread");
  }

  ~scoped_thread() { t.join(); }

  scoped_thread(scoped_thread &) = delete;
  scoped_thread& operator=(scoped_thread const &) = delete;
};

}

namespace undefined_behavior_reference_case {

class Sleeper {
public:
  explicit Sleeper(int &i_) : i{i_} {};

  void operator()(int k) {
    for (unsigned int j = 0; j <= 5; ++j) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      i += k;
    }
    std::cout << std::this_thread::get_id() << std::endl;
  }

private:
  int &i;
};

}

TEST(pass_ref_test, test1) {
  std::cout << std::endl;

  using namespace undefined_behavior_reference_case;

  int val_sleeper = 1000;
  std::thread t(Sleeper(val_sleeper), 5);
  t.join();
  // t.detach(); // undefined-behavior

  std::cout << "val_sleeper = " << val_sleeper << std::endl;

  std::cout << "std::thread::hardware_concurrency() = "
            << std::thread::hardware_concurrency()
            << std::endl;

  std::cout << std::endl;
}

namespace native_handle_test {
// std::thread::native_handle
// Returns the implementation defined underlying thread handle.
// May throw implementation-defined exceptions.

// Example: Use native_handle to enable realtime scheduling of C++ threads on a POSIX system

std::mutex iomutex;

void f(int num) {
  std::this_thread::sleep_for(std::chrono::seconds(1));

  sched_param sch{};
  int policy;
  pthread_getschedparam(pthread_self(), &policy, &sch);
  std::lock_guard<std::mutex> lk(iomutex);

  std::cout << "Thread " << num << " is executing at priority "
            << sch.sched_priority << '\n';
}

} // namespace native_handle_test

TEST(native_handle_case, test1) {
  using namespace native_handle_test;

  std::thread t1(f, 1), t2(f, 2);

  sched_param sch{};
  int policy;
  pthread_getschedparam(t1.native_handle(), &policy, &sch);
  sch.sched_priority = 20;

  if (pthread_setschedparam(t1.native_handle(), SCHED_FIFO, &sch))
    std::cout << "Failed to setschedparam: "
#ifdef __APPLE__
              << std::strerror(errno)
#endif
              << std::endl;

  t1.join();
  t2.join();
}

namespace shared_data_utils {

class Worker {
public:
  explicit Worker(std::string n) : name(std::move(n)) {}

  void operator()() {
    for (int i = 1; i <= 3; ++i) {
      // begin work
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
      // end work
      std::cout << name << ": " << "Work " << i << " done!!!" << std::endl;
    }
  }

private:
  std::string name;
};

} // namespace shared_data_utils

TEST(shared_data_test, test1) {
  std::cout << std::endl;
  std::cout << "Boss: Let's start working.\n\n";

  using namespace shared_data_utils;
  std::thread herb = std::thread(Worker("Herb"));
  std::thread andrei = std::thread(Worker(" Andrei"));
  std::thread scott = std::thread(Worker("   Scott"));

  std::thread bjarne = std::thread(Worker("   Bjarne"));
  std::thread bart = std::thread(Worker("     Bart"));
  std::thread jenne = std::thread(Worker("       Jenne"));

  herb.join();
  andrei.join();
  scott.join();
  bjarne.join();
  bart.join();
  jenne.join();

  std::cout << "\n" << "Boss: Let's go home." << std::endl;

  std::cout << std::endl;
}

namespace unique_lock_utils {

// A std::unique_lock is stronger but more expensive than its little brother std::lock_guard.
// In addition to what's offered by a std::lock_guard, a std::unique_lock enables you to:
// 1. create it without an associated mutex.
// 2. create it without locking the associated mutex.
// 3. explicitly and repeatedly set or release the lock of the associated mutex.
// 4. recursively lock its mutex.
// 5. move the mutex.
// 6. try to lock the mutex.
// 7. delay the lock on the associated mutex.
//
struct critical_data {
  std::mutex mut;
};

void dead_lock(critical_data &a, critical_data &b) {
  std::unique_lock<std::mutex> guard1(a.mut, std::defer_lock);
  std::cout << "Thread: " << std::this_thread::get_id() << " first mutex" << std::endl;

  std::this_thread::sleep_for(std::chrono::milliseconds(1));

  std::unique_lock<std::mutex> guard2(b.mut, std::defer_lock);
  std::cout << " Thread: " << std::this_thread::get_id() << " second mutex" << std::endl;

  std::cout << " Thread: " << std::this_thread::get_id() << " get both mutex" << std::endl;
  std::lock(guard1, guard2);
  // do something with a and b
}

struct bank_account {
  explicit bank_account(std::string name_, int money_)
      : name(std::move(name_)), money(money_)
  {}

  std::string name;
  int money;
  std::mutex mux;
};

void transfer(bank_account &from, bank_account &to, int amount) {
  std::unique_lock<std::mutex> lk1(from.mux, std::defer_lock); // std::defer_lock 表示延迟加锁，此处只管理mutex
  std::unique_lock<std::mutex> lk2(to.mux, std::defer_lock);

  std::lock(lk1, lk2); // lock一次性可以锁住多个 mutex 防止 deadlock
  from.money -= amount;
  to.money += amount;

  std::cout << "Transfer " << amount << " "
            << "from " << from.name << " "
            << "to " << to.name
            << std::endl;
}

} // namespace unique_lock_utils

TEST(unique_lock_test, test1) {
  using namespace unique_lock_utils;

  critical_data c1;
  critical_data c2;

  std::thread t1([&] { dead_lock(c1, c2); });
  std::thread t2([&] { dead_lock(c2, c1); });

  t1.join();
  t2.join();

  std::cout << std::endl;
}

TEST(unique_lock_test, bank_transfer_case) {
  using namespace unique_lock_utils;

  bank_account acc1("User1", 100);
  bank_account acc2("User2", 50);

  std::thread t1([&] () { transfer(acc1, acc2, 10); });
  std::thread t2([&] () { transfer(acc2, acc1, 5); });

  t1.join();
  t2.join();
}

namespace shared_lock_utils {

std::map<std::string, int> tele_book{{"Dijkstra", 1972}, {"Scott", 1976},
                                     {"Ritchie", 1983}};

std::shared_timed_mutex tele_book_mutex;

void add_to_tele_book(const std::string &name, int tele) {
  std::lock_guard<std::shared_timed_mutex> writer_lock(tele_book_mutex);
  std::cout << "\nSTARTING UPDATE " << name;

  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  tele_book[name] = tele;
  std::cout << " ... ENDING UPDATE " << name << std::endl;
}

void print_number(const std::string &name) {
  std::shared_lock<std::shared_timed_mutex> reader_lock(tele_book_mutex);
  std::cout << name << ": " << tele_book[name];
}

} // namespace shared_lock_utils

TEST(shared_lock_test, test1) {
  std::cout << std::endl;

  using namespace shared_lock_utils;

  std::thread reader1([] { print_number("Scott"); });
  std::thread reader2([] { print_number("Ritchie"); });
  std::thread w1([] { add_to_tele_book("Scott", 1968); });
  std::thread reader3([] { print_number("Dijkstra"); });
  std::thread reader4([] { print_number("Scott"); });

  std::thread w2([] { add_to_tele_book("Bjarne", 1965); });
  std::thread reader5([] { print_number("Scott"); });
  std::thread reader6([] { print_number("Ritchie"); });
  std::thread reader7([] { print_number("Scott"); });
  std::thread reader8([] { print_number("Bjarne"); });

  reader1.join();
  reader2.join();
  reader3.join();
  reader4.join();
  reader5.join();
  reader6.join();
  reader7.join();
  reader8.join();

  w1.join();
  w2.join();

  std::cout << std::endl;

  std::cout << "\nThe new telephone book" << std::endl;
  for (auto &tele_item : tele_book)
  {
    std::cout << tele_item.first << ": " << tele_item.second << std::endl;
  }

  std::cout << std::endl;
}
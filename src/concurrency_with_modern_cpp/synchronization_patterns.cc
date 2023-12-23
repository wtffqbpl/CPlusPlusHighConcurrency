#include <gtest/gtest.h>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <new>
#include <string>
#include <shared_mutex>
#include <thread>
#include <utility>

namespace data_race_with_reference_utils {

using namespace std::chrono_literals;

void by_copy(bool b) {
  std::this_thread::sleep_for(1ms);
  std::cout << "by_copy: " << b << std::endl;
}

void by_reference(bool &b) {
  std::this_thread::sleep_for(1ms);
  std::cout << "by_reference: " << b << std::endl;
}

void by_const_reference(const bool &b) {
  std::this_thread::sleep_for(1ms);
  std::cout << "by_const_reference: " << b << std::endl;
}

}

TEST(synchronization_patterns, test1) {
  using namespace data_race_with_reference_utils;

  bool shared{false};

  std::thread t1(by_copy, shared);
  std::thread t2(by_reference, std::ref(shared));
  std::thread t3(by_const_reference, std::cref(shared));

  shared = true;

  t1.join();
  t2.join();
  t3.join();
}

namespace scoped_lock_utils {

class ScopedLock {
private:
  std::mutex &mut;

public:
  explicit ScopedLock(std::mutex &m) : mut(m) {
    mut.lock();
    std::cout << "Lock the mutex: " << &mut << std::endl;
  }

  ~ScopedLock() {
    std::cout << "Release the mutex: " << &mut << std::endl;
    mut.unlock();
  }
};

void test_scoped_lock() {
  std::mutex mutex1;
  ScopedLock scoped_lock1{mutex1};

  std::cout << "\nBefore local scope" << std::endl;
  {
    std::mutex mutex2;
    ScopedLock scoped_lock2{mutex2};
  }
  std::cout << "After local scope" << std::endl;

  std::cout << "\nBefore try-catch block" << std::endl;
  try {
    std::mutex mutex3;
    ScopedLock scoped_lock3{mutex3};
    throw std::bad_alloc();
  }
  catch (std::bad_alloc &e) {
    std::cout << e.what();
  }
  std::cout << "\nAfter try-catch block" << std::endl;
}

} // namespace scoped_lock_utils

TEST(synchronization_patterns, test2) {
  using namespace scoped_lock_utils;
  test_scoped_lock();
}

namespace strategy_pattern_utils {

class Strategy {
public:
  virtual void operator()() = 0;
  virtual ~Strategy() = default;
};

class Context {
  std::shared_ptr<Strategy> _strat;

public:
  explicit Context() : _strat(nullptr) {}

  void set_strategy(std::shared_ptr<Strategy> strat) { _strat = strat; }
  void strategy() { if (_strat) (*_strat)(); }
};


class Strategy1 : public Strategy {
  void operator()() override {
    std::cout << "Foo" << std::endl;
  }
};

class Strategy2 : public Strategy {
  void operator()() override {
    std::cout << "Bar" << std::endl;
  }
};

class Strategy3 : public Strategy {
  void operator()() override {
    std::cout << "FooBar" << std::endl;
  }
};

void test_strategy_policy() {
  Context con;

  con.set_strategy(std::shared_ptr<Strategy>(new Strategy1));
  con.strategy();

  con.set_strategy(std::shared_ptr<Strategy>(new Strategy2));
  con.strategy();

  con.set_strategy(std::shared_ptr<Strategy>(new Strategy3));
  con.strategy();
};

} // namespace strategy_pattern_utils

TEST(synchronization_patterns, test3) {
  using namespace strategy_pattern_utils;

  test_strategy_policy();
}

namespace strategized_locking_runtime {

class Lock {
public:
  virtual void lock() const = 0;
  virtual void unlock() const = 0;
};

class StrategizedLocking {
  Lock &lock;

public:
  StrategizedLocking(Lock &l) : lock(l) {
    lock.lock();
  }

  ~StrategizedLocking() {
    lock.unlock();
  }
};

struct NullObjectMutex {
  void lock() {}
  void unlock() {}
};

class NoLock : public Lock {
  void lock() const override {
    std::cout << "NoLock::lock: " << std::endl;
    null_object_mutex.lock();
  }

  void unlock() const override {
    std::cout << "NoLock::unlock: " << std::endl;
    null_object_mutex.unlock();
  }

private:
  mutable NullObjectMutex null_object_mutex;
};

class ExclusiveLock :public Lock {
  void lock() const override {
    std::cout << "    ExclusiveLock::lock: " << std::endl;
    mtx.lock();
  }

  void unlock() const override {
    std::cout << "    ExclusiveLock::unlock: " << std::endl;
    mtx.unlock();
  }

private:
  mutable std::mutex mtx;
};

class SharedLock : public Lock {
  void lock() const override {
    std::cout << "        SharedLock::lock_shared: " << std::endl;
    shared_mtx.lock();
  }

  void unlock() const override {
    std::cout << "        SharedLock::unlock_shared: " << std::endl;
    shared_mtx.unlock();
  }

private:
  mutable std::shared_mutex shared_mtx;
};

void test_strategized_locking() {
  NoLock no_lock;
  StrategizedLocking strat_lock1{no_lock};

  {
    ExclusiveLock ex_lock;
    StrategizedLocking strat_lock2{ex_lock};
    {
      SharedLock shar_lock;
      StrategizedLocking strat_lock3{shar_lock};
    }
  }
}

} // namespace strategized_locking_runtime

TEST(synchronization_patterns, test4) {
  using namespace strategized_locking_runtime;

  test_strategized_locking();
}

namespace thread_safe_interface {

// This is the simple and straightforward idea:
// 1. All interface methods(public) should use a lock.
// 2. All implementation methods (protected and private) must not use a lock.
// 3. The interface methods call only protected or private methods but no public methods.
//
// The benefits of the thread-safe interface are threefold:
// 1. A recursive call of a mutex is not possible. Recursive calls on a non-recursive
//    mutex are undefined behaviour in C++ and usually end in a deadlock.
// 2. The program uses minimal locking and, therefore, minimal synchronization.
//    Using just a std::recursive_mutex in each public or private method of the
//    class Critical would end in more expensive synchronizations.
// 3. From the user perspective, Critical is straightforward to use because
//    the synchronization is only an implementation detail.

class Critical {
public:
  void interface1() const {
    std::lock_guard<std::mutex> lock_guard(mut);
    implementation1();
  }

  void interface2() const {
    std::lock_guard<std::mutex> lock_guard(mut);
    implementation2();
    implementation3();
    implementation1();
  }

private:
  void implementation1() const {
    std::cout << "implementation1: " << std::this_thread::get_id() << std::endl;
  }

  void implementation2() const {
    std::cout << "implementation2: " << std::this_thread::get_id() << std::endl;
  }

  void implementation3() const {
    std::cout << "implementation3: " << std::this_thread::get_id() << std::endl;
  }

private:
  mutable std::mutex mut;
};

} // namespace thread_safe_interface

TEST(synchronization_patterns, thread_safe_design_test) {
  using namespace thread_safe_interface;

  std::thread t1([] {
    const Critical crit;
    crit.interface1();
  });

  std::thread t2([] {
    Critical crit;
    crit.interface2();
    crit.interface1();
  });

  Critical crit;
  crit.interface1();
  crit.interface2();

  t1.join();
  t2.join();
}

class Base {
public:
  virtual void interface() {
    std::lock_guard<std::mutex> lock_guard(mut);
    std::cout << "Base with lock" << std::endl;
  }

private:
  std::mutex mut;
};

class Derived : public Base {
  void interface() override {
    std::cout << "Derived without lock" << std::endl;
  }
};

TEST(virtuality_issue, test) {
  Base *base1 = new Derived;
  base1->interface();

  Derived der;
  Base &base2 = der;
  base2.interface();
}
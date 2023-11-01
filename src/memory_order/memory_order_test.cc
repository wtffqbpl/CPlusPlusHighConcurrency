#include <gtest/gtest.h>
#include <algorithm>
#include <atomic>
#include <iostream>
#include <iterator>
#include <thread>

namespace {

std::atomic<int> a{0};

void thread1() {
  for (int i = 0; i < 10; i += 2)
  {
    a.store(i, std::memory_order_relaxed);
  }
}

void thread2() {
  for (int i = 1; i < 10; i += 2)
  {
    a.store(i, std::memory_order_relaxed);
  }
}

void thread3(std::vector<int> *v) {
  for (int i = 0; i < 10; ++i)
  {
    v->push_back(a.load(std::memory_order_relaxed));
  }
}

void thread4(std::vector<int> *v) {
  for (int i = 0; i < 10; ++i)
  {
    v->push_back(a.load(std::memory_order_relaxed));
  }
}

void memory_order_relaxed_test1() {
  std::vector<int> v3, v4;
  std::thread t1(thread1), t2(thread2) , t3(thread3, &v3), t4(thread4, &v4);
  t1.join();
  t2.join();
  t3.join();
  t4.join();

  std::copy(v3.begin(), v3.end(), std::ostream_iterator<int>(std::cout, " "));
  std::cout << std::endl;

  std::copy(v4.begin(), v4.end(), std::ostream_iterator<int>(std::cout, " "));
  std::cout << std::endl;
}

// The happens-before relationship
// The happens-before and strongly-happens-before relationships are the basic building blocks
// of operation ordering in a program; it specifies which operations see the effects of
// which other operations.
// For a single thread, it's largely straightforward: if one operation is sequenced
// before another, then it also happens before it, and strongly-happens-before it.
// This means that if one operation (A) occurs in a statement prior to another (B)
// in the source code, then A happens before B, and A strongly-happens-before B.
// If the operations occur in the same statement, in general there's no happens-before
// relationship between them, because they're unordered. This is another way of
// saying that the ordering is unspecified.

// Sequentially Consistent Ordering

std::atomic<bool> x, y;
std::atomic<int> z;

void write_x() {
  x.store(true, std::memory_order_seq_cst);
}

void write_y() {
  y.store(true, std::memory_order_seq_cst);
}

void read_x_then_y() {
  while (!x.load(std::memory_order_seq_cst));
  if (y.load(std::memory_order_seq_cst))
    ++z;
}

void read_y_then_x() {
  while (!y.load(std::memory_order_seq_cst));
  if (x.load(std::memory_order_seq_cst))
    ++z;
}

int sequentially_consistent_ordering_test() {
  x = false;
  y = false;
  z = 0;
  std::thread a(write_x);
  std::thread b(write_y);
  std::thread c(read_x_then_y);
  std::thread d(read_y_then_x);

  a.join();
  b.join();
  c.join();
  d.join();

  return z.load();
}

std::atomic<bool> x_relaxed, y_relaxed;
std::atomic<int> z_relaxed;

void write_x_then_y_relaxed() {
  x.store(true, std::memory_order_relaxed);
  y.store(true, std::memory_order_relaxed);
}

void read_y_then_x_relaxed() {
  while (!y.load(std::memory_order_relaxed));
  if (x.load(std::memory_order_relaxed))
    ++z;
}

int relaxed_memory_order_test1() {
  x = false;
  y = false;
  z = 0;

  std::thread a(write_x_then_y_relaxed);
  std::thread b(read_y_then_x);
  a.join();
  b.join();

  return z.load();
}

std::atomic<int> x_relaxed_2(0), y_relaxed_2(0), z_relaxed_2(0);
std::atomic<bool> go(false);

constexpr int loop_count = 10;

struct read_values {
  int x, y, z;
};

read_values values1[loop_count];
read_values values2[loop_count];
read_values values3[loop_count];
read_values values4[loop_count];
read_values values5[loop_count];

void increment(std::atomic<int> *var_to_inc, read_values *values) {
  while (!go)
    std::this_thread::yield();

  for (unsigned i = 0; i < loop_count; ++i) {
    values[i].x = x_relaxed_2.load(std::memory_order_relaxed);
    values[i].y = y_relaxed_2.load(std::memory_order_relaxed);
    values[i].z = z_relaxed_2.load(std::memory_order_relaxed);

    var_to_inc->store(i + 1, std::memory_order_relaxed);
    std::this_thread::yield();
  }
}

void read_vals(read_values *values) {
  while (!go)
    std::this_thread::yield();

  for (unsigned i = 0; i < loop_count; ++i) {
    values[i].x = x_relaxed_2.load(std::memory_order_relaxed);
    values[i].y = y_relaxed_2.load(std::memory_order_relaxed);
    values[i].z = z_relaxed_2.load(std::memory_order_relaxed);
    std::this_thread::yield();
  }
}

void print(read_values *v) {
  for (unsigned  i = 0; i < loop_count; ++i) {
    if (i) std::cout << ",";
    std::cout << "(" << v[i].x << "," << v[i].y << "," << v[i].z << ")";
  }
  std::cout << std::endl;
}

void memory_order_relaxed_test2() {
  std::thread t1(increment, &x_relaxed_2, values1);
  std::thread t2(increment, &y_relaxed_2, values2);
  std::thread t3(increment, &z_relaxed_2, values3);
  std::thread t4(read_vals, values4);
  std::thread t5(read_vals, values5);

  go = true;

  t5.join();
  t4.join();
  t3.join();
  t2.join();
  t1.join();

  print(values1);
  print(values2);
  print(values3);
  print(values4);
  print(values5);
}

// memory_order_release/memory_order_acquire

namespace acquire_release_memory_order {

std::atomic<bool> x, y;
std::atomic<int> z;

void write_x() {
  x.store(true, std::memory_order_release);
}

void write_y() {
  y.store(true, std::memory_order_release);
}

void read_x_then_y() {
  while (!x.load(std::memory_order_acquire));
  if (y.load(std::memory_order_acquire))
    ++z;
}

void read_y_then_x() {
  while (!y.load(std::memory_order_acquire));
  if (x.load(std::memory_order_acquire))
    ++z;
}

int test() {
  x = false;
  y = false;
  z = 0;

  std::thread a(write_x);
  std::thread b(write_y);
  std::thread c(read_x_then_y);
  std::thread d(read_y_then_x);

  a.join();
  b.join();
  c.join();
  d.join();

  return z.load();
}
}

}

TEST(memory_order_test, relaxed_test1) {
  memory_order_relaxed_test1();
}

TEST(memory_order_test, sequential_order_test1) {
  auto value_z = sequentially_consistent_ordering_test();

#ifndef NDEBUG
  std::cout << "value z: " << value_z << std::endl;
#endif

  EXPECT_NE(value_z, 0);
}

TEST(memory_order_test, relaxed_memory_order_test1) {
  auto value_z_relaxed = relaxed_memory_order_test1();

#ifndef NDEBUG
  std::cout << "value z (relaxed): " << value_z_relaxed << std::endl;
#endif

  // value_z_relaxed may be 0 since x.store() and y.store() is memory_order_relaxed,
  // so maybe y.store() is happened before x.store(), although x.store() statement
  // is before y.store().
  // EXPECT_NE(value_z_relaxed, 0);
}

TEST(memory_order_test, relaxed_memory_order_test2) {
  memory_order_relaxed_test2();
}

TEST(memory_order_test, acquire_release_order_test1) {
  auto value_z = acquire_release_memory_order::test();

  std::cout << "value z: " << value_z << std::endl;
}
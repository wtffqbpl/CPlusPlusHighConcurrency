#include <gtest/gtest.h>
#include <atomic>
#include <thread>

namespace {

std::atomic<int> a{0};

void thread1() { a = 1; }

void thread2() {
  std::cout << "a = " << a << std::endl;
  std::flush(std::cout);
}

void basics_test1() {
  std::thread t1(thread1);
  std::thread t2(thread2);

  t1.join();
  t2.join();
}

// 原子变量的操作
// 对于原子变量的操作可以分为三种:
// 1. store: 将一个值存储到原子变量中
// 2. load: 读取原子变量中的值
// 3. read-modify-write(RMW). 原子地执行读取、修改和写入。 如 fetch_add, exchange等
// 每个原子操作都需要指定一个内存顺序(memory order). 不同的内存顺序有不同的语意，会实现
// 不同的顺序模型(order model), 性能也不同.

// enum memory_order {
//   memory_order_relaxed,
//   memory_order_consume,
//   memory_order_acquire,
//   memory_order_release,
//   memory_order_acq_rel,
//   memory_order_seq_cst,
// };


}

TEST(basics_test, test1) {
  basics_test1();
}

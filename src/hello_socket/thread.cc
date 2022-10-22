#include "gtest/gtest.h"
#include <iostream>
#include <thread>

void thread_task() {
    std::cout << "hello thread. Thread Id is "
              << std::this_thread::get_id() << std::endl;
}

TEST(hello_socket, demo) {
    std::thread t(thread_task);
    t.join();
}
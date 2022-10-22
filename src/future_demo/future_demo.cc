#include "gtest/gtest.h"
#include <chrono>
#include <future>
#include <iostream>

int find_the_answer_to_ltuae() {
    return 42;
}

void do_other_stuff() {
    std::cout << "Hello world!" << std::endl;
}

TEST(future_test, demo) {
    std::future<int> the_answer = std::async(find_the_answer_to_ltuae);
    the_answer.wait();
}
#include <future>
#include <iostream>
#include <chrono>

int find_the_answer_to_ltuae()
{
    return 42;
    ch
}

void do_other_stuff()
{
    std::cout << "Hello world!" << std::endl;
}

int main()
{
    std::future<int> the_answer = std::async(find_the_answer_to_ltuae);
}

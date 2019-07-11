#include <thread>
#include <mutex>
#include <chrono>
#include <iostream>
#include <vector>
#include <stdexcept>

std::mutex mtx;          // locks access to counter


void print_block(int charNum, char c)
{
    std::unique_lock<std::mutex> lck(mtx);

    for (size_t charIdx = 0; charIdx < charNum; ++charIdx) {
        std::cout << c;
    }
    std::cout << std::endl;
}

int main(int argc, char* argv[])
{
    std::thread th1(print_block, 50, '*');
    std::thread th2(print_block, 50, '$');

    th1.join();
    th2.join();

    return 0;
}


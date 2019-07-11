#include <thread>
#include <mutex>
#include <chrono>
#include <iostream>
#include <vector>
#include <stdexcept>

#define MAX_THREAD_NUM    (10)

std::mutex mtx;          // locks access to counter


void print_even(int x)
{
    if (x % 2 == 0) {
        std::cout << x << " is even\n";
    } else {
        throw (std::logic_error("not even"));
    }
}

void print_thread_id(int id)
{
    try {
        // using a local lock_guard to lock mtx guarantees unlocking on destruction / exception.
        std::lock_guard<std::mutex> lck(mtx);
        print_even(id);
    }
    catch(std::logic_error&) {
        std::cout << "[exception caught]\n";
    }
}

/*********************************************************************************
 *
 *     NAME:   main
 *  Description: program entry routine.
 *
 *********************************************************************************/
int main(int argc, char* argv[])
{
    std::vector<std::thread> threads;
    threads.reserve(MAX_THREAD_NUM);

    for (size_t threadIdx = 0; threadIdx < MAX_THREAD_NUM; ++threadIdx) {
        threads.push_back(std::thread(print_thread_id, threadIdx+1));
    }

    for (auto& th: threads) th.join();

    return 0;

}

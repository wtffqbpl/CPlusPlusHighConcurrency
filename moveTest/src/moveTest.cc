#include <iostream>
#include <thread>
#include <chrono>


void thread_task(int n)
{
    std::this_thread::sleep_for(std::chrono::seconds(n));
    std::cout << "Hello thread "
        << std::this_thread::get_id()
        << " pause " << n << " seconds. " << std::endl;
}


int main(int argc, char* argv[])
{
    std::thread threads[5];

    std::cout << "Spawning 5 threads..." << std::endl;

    for (size_t idx = 0; idx < 5; ++idx) {
        threads[idx] = std::thread(thread_task, idx + 1);
    }

    std::cout << "Done spawning threads! Now wait for them to join." << std::endl;

    for (auto& t: threads) {
        t.join();
    }

    std::cout << "All threads joined." << std::endl;

    return EXIT_SUCCESS;
}

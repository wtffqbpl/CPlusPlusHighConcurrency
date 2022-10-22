#include <gtest/gtest.h>
#include <iostream>
#include <future>
#include <chrono>
#include <thread>

namespace {
int countdown(int from, int to)
{
    for (size_t i = from; i != to; --i) {
        std::cout << i << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "Finished!" << std::endl;

    return from - to;
}

}

TEST(packaged_task, demo) {
    std::packaged_task<int(int, int)> task(countdown);  // 设置packaged_task
    std::future<int> ret = task.get_future();  // 获得与packaged_task共享状态相关联的fu

    std::thread th(std::move(task), 10, 0);   // 创建一个新线程完成计数任务

    int value = ret.get();                      // 等待任务完成并获取结果

    std::cout << "The countdown lasted for " << value << "seconds.\n";

    th.join();
}
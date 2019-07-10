#include <stdio.h>
#include <stdlib.h>

#include <thread>
#include <iostream>


void thread_task()
{
    // std::cout << "hello thread. Thread Id is " << std::this_thread.get_id() << std::endl;
    std::cout << "hello thread." << std::endl;
}


/*********************************************************************************
 *
 *     NAME:   main
 *  Description: program entry routine.
 *
 *********************************************************************************/
int main(int argc, char* argv[])
{
    std::thread t(thread_task);

    t.join();

    return 0;
}

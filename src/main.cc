#include <iostream>
#include "my_threadPool.h"

// io 互斥量
std::mutex mtx_io;

void func(int i, int j)
{
    {
        std::unique_lock<std::mutex> ulock(mtx_io);
        std::cout << "task" << i + j << "begin to work...\n";
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
    {
        std::unique_lock<std::mutex> ulock(mtx_io);
        std::cout << "task" << i + j << "done...\n";
    }
}

int main()
{
    threadPool tp(2);

    for (int i = 0; i < 10; ++i)
    {
        /*tp.append([i]() {
            {
                std::unique_lock<std::mutex> ulock(mtx_io);
                std::cout << "task" << i << "begin to work...\n";
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
            {
                std::unique_lock<std::mutex> ulock(mtx_io);
                std::cout << "task" << i << "done...\n";
            }
            });*/

        tp.append_task(func, i, 10);
    }

    return 0;
}

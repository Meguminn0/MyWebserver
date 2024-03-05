#include "my_threadPool.h"

threadPool::threadPool(int cnt) noexcept : stop(false)
{
    // 初始化其他线程
    while (cnt--)
    {
        all_threads.emplace_back(std::thread(thread_doWork, this));
    }
}

threadPool::~threadPool()
{
    std::unique_lock<std::mutex> unilock(que_mtx);
    stop = true;
    unilock.unlock();

    // 唤醒所有线程
    cv.notify_all();

    // 等待所有线程关闭
    for (auto& item : all_threads)
    {
        item.join();
    }
}

void threadPool::thread_doWork(void* argv)
{
    threadPool* tdpool = (threadPool*)argv;
    while (true)
    {
        // 访问线程池的任务队列，进行加锁，防止线程间争抢
        // 同时只能有一个线程拿到任务队列权限，其余线程在此阻塞等待
        std::unique_lock<std::mutex> unilock(tdpool->que_mtx);

        /* 拿到了任务队列的权限 */

        // 如果任务队列为空，并且没有关闭线程池，等待任务
        while (tdpool->que_task.empty() && !tdpool->stop)
        {
            // 如果使用这行语句，可以不用循环
            // 循环的目的是为了防止 **虚假唤醒**
            /*
            虚假循环：
                进程被唤醒后本可以获取对应的资源，但有时发现并没有对应的资源，
                可能的原因是在他从阻塞队列到就绪队列的过程中，恰好有一个就绪队列
                中的线程开始去获取资源，并被他抢夺 )。
                如果我们没有做对应的处理，可能会导致未定义行为(程序的结果与预期不符)。
                因此我们应当遵循循环检测条件，当进程被唤醒时，立即重新检查条件，
                并在条件不满足时再次进入阻塞状态 ( 重新进入阻塞队列 )。
                `wait(unique_lock<mutex>& _Lck, _Predicate _Pred)` 在函数内使用了循环检测。
            */
            // tdpool->cv.wait(unilock, [tdpool]() -> bool { return  !tdpool->que_task.empty() || tdpool->stop; });

            tdpool->cv.wait(unilock);
        }

        // 当关闭了线程池并且池中任务都完成后才关闭线程
        if (tdpool->stop && tdpool->que_task.empty())
        {
            // 放弃任务队列的权限
            unilock.unlock();
            // 关闭线程
            return;
        }

        // 拿到任务队列中的任务
        std::function<void()> func(std::move(tdpool->que_task.front()));
        tdpool->que_task.pop();
        // 放弃任务队列的权限
        unilock.unlock();

        // 执行任务
        func();
    }
}
#ifndef __MY_THREADPOOL_H__
#define __MY_THREADPOOL_H__

#include <thread>
#include <vector>
#include <queue>
#include <functional>

#include <thread>
#include <mutex>
#include <condition_variable>

#define _MIN_THREADS 4
#define _MAX_THREADS 32
#define _THREAD_ADD_NUM_STEP 4
#define _THREAD_DELETE_NUM_STEP 2

class threadPool
{
public:
    threadPool(int cnt) noexcept;
    ~threadPool();

    // 向池中加入队列
    template<typename F, typename... Args>
    bool append_task(F&& _f, Args&&... _args);

protected:
    static void thread_doWork(void* argv);

private:
    // 禁止拷贝构造
    threadPool(const threadPool&) = delete;
    threadPool& operator=(const threadPool&) = delete;

    std::vector<std::thread> all_threads;
    std::queue<std::function<void()>> que_task;

    std::mutex que_mtx;         /* 任务队列互斥量 */
    std::condition_variable  cv;
    bool stop;                  /* 关闭线程池 */
};

template<typename F, typename... Args>
bool threadPool::append_task(F&& _f, Args&&... _args)
{
    if (stop)
    {
        return false;
    }

    // 将任务绑定，并统一为 std::function<void> 的形式 
    std::function<void()> func(std::bind(std::forward<F>(_f), std::forward<Args>(_args)...));

    // 获取任务队列权限
    std::unique_lock<std::mutex> unilock(que_mtx);
    que_task.push(std::move(func));
    // 放弃任务队列的权限
    unilock.unlock();
    // 唤醒一个等待队列中的线程
    cv.notify_one();

    return true;
}

#endif
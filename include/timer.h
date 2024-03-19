#ifndef TIMER_H
#define TIMER_H

#include <functional>
#include <chrono>

// 定时器类
class timer
{
friend class timerManager;

public:
    timer(long long seconds);
    timer(long long seconds, int rep);
    ~timer() {};

    template<typename F, typename... Args>
    void callBack_func(F &&fn, Args &&... args);

    void on_time();     /* 执行定时任务 */
    void setExpireTIme(time_t newTime);     /* 重新设置到期时间点 */
    time_t getNow_sec();     /* 获取当前时间，单位：秒 */

public:
    std::function<void()> action_func;

private:
    time_t expire_time;     /* 到期时间点 */
    int repeat;             /* 定时次数，-1为无限次 */
    long long period;       /* 定时间隔，单位：秒 */

    // 用双向列链表实现定时器
    timer *prev;
    timer *next;
};


template<typename F, typename... Args>
void timer::callBack_func(F &&fn, Args &&... args)
{
    action_func = std::bind(std::forward<F>(fn), std::forward<Args>(args)...);
}

// 定时器管理类，使用双向链表实现
class timerManager
{
public:
    timerManager() : head(nullptr), tail(nullptr) {}
    ~timerManager();

    bool add_timer(timer *addTimer);        /* 加入定时器 */
    bool delete_timer(timer *delTimer);     /* 删除定时器 */
    void tick();                            /* 执行到达时间的定时任务 */
    bool adjust_timer(timer *adjTimer);     /* 调整定时器 */

private:
    bool add_mid_timer(timer *addTimer, timer *point);

private:
    timer *head;
    timer *tail;
};

#endif
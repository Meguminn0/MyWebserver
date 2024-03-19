#include "timer.h"

timer::timer(long long seconds) :  period(seconds), repeat(-1), prev(nullptr), next(nullptr)
{
    // 获取当前的系统时间，单位：秒
    // 然后将定时间隔和当前系统时间的和作为第一次定时触发的时间
    auto now = std::chrono::system_clock::now();
    auto now_sec = std::chrono::time_point_cast<std::chrono::seconds>(now);
    expire_time = getNow_sec() + seconds;
}

timer::timer(long long seconds, int rep) : timer(seconds)
{
    repeat = rep;
}

void timer::on_time()
{
    // 执行定时任务
    if(!action_func || repeat == 0)
    {
        return;
    }

    action_func();
    expire_time += period;
    if(repeat > 0)
    {
        --repeat;
    }
}

void timer::setExpireTIme(time_t newTime)
{
    expire_time = newTime;
}

time_t timer::getNow_sec()
{
    auto now = std::chrono::system_clock::now();
    auto now_sec = std::chrono::time_point_cast<std::chrono::seconds>(now);
    return now_sec.time_since_epoch().count();
}

timerManager::~timerManager()
{
    timer *tmp = head;
    if(tmp)
    {
        head = tmp->next;
        delete tmp;
        tmp = head;
    }
}

bool timerManager::add_timer(timer *addTimer)
{
    if(!addTimer)
    {
        return false;
    }

    if(!head)
    {
        head = tail = addTimer;
        return true;
    }

    if(addTimer->expire_time <= head->expire_time)
    {
        head->prev = addTimer;
        addTimer->next = head;
        head = addTimer;
        return true;
    }
    return add_mid_timer(addTimer, head->next);
}

bool timerManager::delete_timer(timer *delTimer)
{
    if(!delTimer)
    {
        return false;
    }

    if(delTimer->prev == nullptr && delTimer->next == nullptr)
    {
        head = tail = nullptr;
        delete delTimer;
        return true;
    }
    else if(delTimer == head)
    {
        head = head->next;
        head->prev = nullptr;
        delete delTimer;
        return true;
    }
    else if(delTimer == tail)
    {
        tail = delTimer->prev;
        tail->next = nullptr;
        delete delTimer;
        return true;
    }
   
    delTimer->next->prev = delTimer->prev;
    delTimer->prev->next = delTimer->next;
    delete delTimer;
    return true;
}

void timerManager::tick()
{
    if(!head)
    {
        return;
    }

    timer *tmp = head;
    long long now = tmp->getNow_sec();
    while(tmp)
    {
        if(now < tmp->expire_time)
        {
            break;
        }
        tmp->on_time();
        if(tmp->repeat > 0 || tmp->repeat == -1)
        {
            adjust_timer(tmp);
        }
        else
        {
            delete_timer(tmp);
        }

        tmp = head;
    }
}

bool timerManager::adjust_timer(timer *adjTimer)
{
    if(!adjTimer)
    {
        return false;
    }

    if(!adjTimer->next || adjTimer->expire_time <= adjTimer->next->expire_time)
    {
        return true;
    }

    if(adjTimer == head)
    {
        head = adjTimer->next;
        head->prev = nullptr;
    }
    else
    {
        adjTimer->next->prev = adjTimer->prev;
        adjTimer->prev->next = adjTimer->next;
    }

    return add_mid_timer(adjTimer, adjTimer->next);
}

bool timerManager::add_mid_timer(timer *addTimer, timer *point)
{
    while(point)
    {
        if(addTimer->expire_time <= point->expire_time)
        {
            addTimer->next = point;
            addTimer->prev = point->prev;
            point->prev->next = addTimer;
            point->prev = addTimer;
            return true;
        }
        point = point->next;
    }

    addTimer->next = tail->next;
    addTimer->prev = tail;
    tail->next = addTimer;
    tail = addTimer;
    return true;
}

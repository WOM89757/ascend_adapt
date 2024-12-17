#ifndef __TIMER_H__
#define __TIMER_H__

#include <iostream>
#include <thread>
#include <functional>
#include <atomic>
#include <chrono>

class Timer
{
public:
    Timer() : is_running(false) {}
    Timer(std::function<void()> task, std::chrono::milliseconds interval);

    ~Timer();

    // 启动定时器
    void start(std::function<void()> task, std::chrono::milliseconds interval);

    // 停止定时器
    void stop();

    // 检查定时器是否正在运行
    bool running() const;

private:
    std::atomic<bool> is_running; // 定时器运行状态
    std::thread timer_thread;     // 定时线程
};

#endif /* __TIMER_H__ */
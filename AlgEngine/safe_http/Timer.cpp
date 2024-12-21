#include "Timer.h"

Timer::Timer(std::function<void()> task, std::chrono::milliseconds interval)
    : is_running(false)
{
    start(task, interval);
}

Timer::~Timer() { stop(); }

// 启动定时器
void Timer::start(std::function<void()> task,
                  std::chrono::milliseconds interval)
{
    if (is_running.load())
    {
        std::cerr << "Timer is already running!" << std::endl;
        return;
    }

    is_running.store(true);

    timer_thread = std::thread([this, task, interval]() {
        while (is_running.load())
        {
            auto start_time = std::chrono::steady_clock::now();
            try
            {
                task();
            }
            catch (const std::exception& e)
            {
                std::cerr << "Timer task exception: " << e.what() << std::endl;
            }

            auto end_time = std::chrono::steady_clock::now();
            std::this_thread::sleep_for(interval - (end_time - start_time));
        }
    });
}

// 停止定时器
void Timer::stop()
{
    is_running.store(false);
    if (timer_thread.joinable())
    {
        timer_thread.join(); // 等待线程结束
    }
}

// 检查定时器是否正在运行
bool Timer::running() const { return is_running.load(); }

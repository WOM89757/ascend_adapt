#ifndef __THREADBASE_H__
#define __THREADBASE_H__

#include <iostream>
#include <thread>
#include <atomic>
#include <functional>

class ThreadBase
{
public:
    ThreadBase() : threadRunning(false) {}

    virtual ~ThreadBase()
    {
        stopThr(); // 确保线程在析构时结束
    }

    // 启动线程
    void startThr()
    {
        if (threadRunning)
        {
            std::cerr << "Thread is already running!" << std::endl;
            return;
        }
        threadRunning = true;
        workerThread = std::thread(&ThreadBase::run, this);
    }

    // 停止线程
    void stopThr()
    {
        if (threadRunning)
        {
            threadRunning = false; // 通知线程停止
            if (workerThread.joinable())
            {
                workerThread.join();
            }
        }
    }

    // 检查线程是否在运行
    bool isRunning() const { return threadRunning; }

protected:
    // 线程任务函数（需要子类实现）
    virtual void run() = 0;

private:
    std::thread workerThread;        // 工作线程
    std::atomic<bool> threadRunning; // 标记线程是否在运行
};

#endif /* __THREADBASE_H__ */
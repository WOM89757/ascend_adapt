#ifndef __THREADSAFEQUEUE_H__
#define __THREADSAFEQUEUE_H__

#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

template <typename T>
class ThreadSafeQueue {
public:
    ThreadSafeQueue() = default;
    ~ThreadSafeQueue() = default;

    // 禁用拷贝和赋值
    ThreadSafeQueue(const ThreadSafeQueue&) = delete;
    ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;

    // 添加元素到队列
    void push(const T& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(value);
        cond_var_.notify_one(); // 通知等待的线程
    }

    void push(T&& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(std::move(value));
        cond_var_.notify_one(); // 通知等待的线程
    }

    // 取出队列中的元素（阻塞）
    T pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_var_.wait(lock, [this]() { return !queue_.empty(); }); // 等待队列非空
        T value = std::move(queue_.front());
        queue_.pop();
        return value;
    }

    // 取出队列中的元素（非阻塞，返回 std::optional）
    std::optional<T> try_pop() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) {
            return std::nullopt; // 队列为空
        }
        T value = std::move(queue_.front());
        queue_.pop();
        return value;
    }

    // 检查队列是否为空
    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    // 获取队列大小
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

private:
    mutable std::mutex mutex_;              // 用于保护队列的互斥锁
    std::condition_variable cond_var_;     // 条件变量，用于等待
    std::queue<T> queue_;                  // 底层队列
};

#endif /* __THREADSAFEQUEUE_H__ */
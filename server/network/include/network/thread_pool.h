#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>

namespace network {

class ThreadPool {
public:
    ThreadPool(int thread_count = 4);
    ~ThreadPool();

    // 提交任务
    void submit(std::function<void()> task);

    // 停止线程池
    void stop();

private:
    void worker();

private:
    std::vector<std::thread> threads_;
    std::queue<std::function<void()>> tasks_;
    std::mutex mutex_;
    std::condition_variable condition_;
    bool stop_;
};

} // namespace network

#endif // THREAD_POOL_H
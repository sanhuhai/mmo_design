#ifndef OBJECT_POOL_H
#define OBJECT_POOL_H

#include <vector>
#include <mutex>
#include <functional>

namespace network {

template <typename T>
class ObjectPool {
public:
    ObjectPool(size_t initial_size = 1024) {
        for (size_t i = 0; i < initial_size; ++i) {
            pool_.push_back(new T());
        }
    }

    ~ObjectPool() {
        for (auto obj : pool_) {
            delete obj;
        }
        for (auto obj : used_) {
            delete obj;
        }
    }

    // 从池获取对象
    T* acquire() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!pool_.empty()) {
            T* obj = pool_.back();
            pool_.pop_back();
            used_.push_back(obj);
            return obj;
        }
        
        // 没有可用对象，创建新对象
        T* obj = new T();
        used_.push_back(obj);
        return obj;
    }

    // 归还对象到池
    void release(T* obj) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = std::find(used_.begin(), used_.end(), obj);
        if (it != used_.end()) {
            used_.erase(it);
            pool_.push_back(obj);
        }
    }

    // 获取当前使用的对象数
    size_t getUsedCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return used_.size();
    }

    // 获取池中的对象数
    size_t getPoolCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return pool_.size();
    }

private:
    std::vector<T*> pool_;
    std::vector<T*> used_;
    mutable std::mutex mutex_;
};

} // namespace network

#endif // OBJECT_POOL_H
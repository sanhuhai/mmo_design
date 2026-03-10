#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#include <vector>
#include <mutex>
#include <cstddef>

namespace network {

class MemoryPool {
private:
    struct MemoryBlock {
        char* data;
        bool used;
    };

public:
    MemoryPool(size_t block_size, size_t block_count = 1024);
    ~MemoryPool();

    void* allocate();
    void deallocate(void* ptr);
    size_t getBlockSize() const;
    size_t getUsedBlocks() const;
    size_t getTotalBlocks() const;

private:
    size_t block_size_;
    size_t block_count_;
    std::vector<MemoryBlock> blocks_;
    mutable std::mutex mutex_;
};

} // namespace network

#endif // MEMORY_POOL_H
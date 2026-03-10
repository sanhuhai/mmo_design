#include "network/memory_pool.h"

namespace network {

MemoryPool::MemoryPool(size_t block_size, size_t block_count) 
    : block_size_(block_size), block_count_(block_count) {
    blocks_.reserve(block_count);
    for (size_t i = 0; i < block_count; ++i) {
        MemoryBlock block;
        block.data = new char[block_size];
        block.used = false;
        blocks_.push_back(block);
    }
}

MemoryPool::~MemoryPool() {
    for (auto& block : blocks_) {
        delete[] block.data;
    }
}

void* MemoryPool::allocate() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (auto& block : blocks_) {
        if (!block.used) {
            block.used = true;
            return block.data;
        }
    }
    
    MemoryBlock block;
    block.data = new char[block_size_];
    block.used = true;
    blocks_.push_back(block);
    block_count_++;
    
    return block.data;
}

void MemoryPool::deallocate(void* ptr) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (auto& block : blocks_) {
        if (block.data == ptr) {
            block.used = false;
            break;
        }
    }
}

size_t MemoryPool::getBlockSize() const {
    return block_size_;
}

size_t MemoryPool::getUsedBlocks() const {
    std::lock_guard<std::mutex> lock(mutex_);
    size_t count = 0;
    for (const auto& block : blocks_) {
        if (block.used) {
            count++;
        }
    }
    return count;
}

size_t MemoryPool::getTotalBlocks() const {
    return block_count_;
}

} // namespace network
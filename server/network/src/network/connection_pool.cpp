#include "network/connection_pool.h"
#include <uv.h>

namespace network {

ConnectionPool::ConnectionPool(int max_connections) : max_connections_(max_connections) {
}

ConnectionPool::~ConnectionPool() {
    // 清理所有连接
    for (auto conn : free_connections_) {
        delete conn;
    }
    for (auto conn : used_connections_) {
        delete conn;
    }
}

uv_tcp_t* ConnectionPool::acquire() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!free_connections_.empty()) {
        uv_tcp_t* conn = free_connections_.back();
        free_connections_.pop_back();
        used_connections_.push_back(conn);
        return conn;
    }
    
    if (used_connections_.size() < max_connections_) {
        uv_tcp_t* conn = new uv_tcp_t();
        used_connections_.push_back(conn);
        return conn;
    }
    
    return nullptr;
}

void ConnectionPool::release(uv_tcp_t* conn) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = std::find(used_connections_.begin(), used_connections_.end(), conn);
    if (it != used_connections_.end()) {
        used_connections_.erase(it);
        free_connections_.push_back(conn);
    }
}

int ConnectionPool::getCurrentConnections() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return used_connections_.size() + free_connections_.size();
}

int ConnectionPool::getMaxConnections() const {
    return max_connections_;
}

} // namespace network
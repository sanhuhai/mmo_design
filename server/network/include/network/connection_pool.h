#ifndef CONNECTION_POOL_H
#define CONNECTION_POOL_H

#include <vector>
#include <mutex>
#include <uv.h>

namespace network {

class ConnectionPool {
public:
    ConnectionPool(int max_connections = 1000);
    ~ConnectionPool();

    // 从池获取连接
    uv_tcp_t* acquire();

    // 归还连接到池
    void release(uv_tcp_t* conn);

    // 获取当前连接数
    int getCurrentConnections() const;

    // 获取最大连接数
    int getMaxConnections() const;

private:
    std::vector<uv_tcp_t*> free_connections_;
    std::vector<uv_tcp_t*> used_connections_;
    std::mutex mutex_;
    int max_connections_;
};

} // namespace network

#endif // CONNECTION_POOL_H
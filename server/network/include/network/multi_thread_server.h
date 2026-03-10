#ifndef MULTI_THREAD_SERVER_H
#define MULTI_THREAD_SERVER_H

#include <uv.h>
#include <memory>
#include <unordered_map>
#include "connection.h"
#include "thread_pool.h"

namespace network {

class MultiThreadServer {
public:
    MultiThreadServer(int io_threads = 1, int worker_threads = 4);
    ~MultiThreadServer();

    // 启动服务器
    bool start(const std::string& host, int port);

    // 停止服务器
    void stop();

    // 设置连接回调
    void setConnectionCallback(ConnectionCallback callback);

    // 设置消息回调
    void setMessageCallback(MessageCallback callback);

    // 设置错误回调
    void setErrorCallback(ErrorCallback callback);

    // 发送消息给指定连接
    bool send(uv_tcp_t* conn, const std::string& message);

    // 关闭连接
    void close(uv_tcp_t* conn);

private:
    static void onNewConnection(uv_stream_t* server, int status);
    static void onAllocBuffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);
    static void onRead(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);
    static void onWrite(uv_write_t* req, int status);
    static void onClose(uv_handle_t* handle);

private:
    uv_tcp_t server_;
    std::vector<uv_loop_t*> io_loops_;
    std::vector<std::thread> io_threads_;
    std::unique_ptr<ThreadPool> worker_pool_;
    ConnectionCallback connection_callback_;
    MessageCallback message_callback_;
    ErrorCallback error_callback_;
    std::unordered_map<uv_tcp_t*, std::unique_ptr<Connection>> connections_;
    std::mutex connections_mutex_;
};

} // namespace network

#endif // MULTI_THREAD_SERVER_H
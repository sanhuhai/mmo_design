#ifndef SERVER_H
#define SERVER_H

#include "network/network.h"
#include "network/connection.h"
#include <uv.h>
#include <string>
#include <functional>
#include <unordered_map>
#include <memory>

namespace network {

using ConnectionCallback = std::function<void(uv_tcp_t*)>;
using MessageCallback = std::function<void(const std::string&)>;
using ErrorCallback = std::function<void(ErrorCode)>;

class Server {
public:
    Server();
    ~Server();

    bool start(const std::string& host, int port);
    void stop();

    void setConnectionCallback(ConnectionCallback callback);
    void setMessageCallback(MessageCallback callback);
    void setErrorCallback(ErrorCallback callback);

    bool send(uv_tcp_t* conn, const std::string& message);
    void close(uv_tcp_t* conn);

private:
    static void onNewConnection(uv_stream_t* server, int status);
    static void onAllocBuffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);
    static void onRead(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);
    static void onWrite(uv_write_t* req, int status);
    static void onClose(uv_handle_t* handle);

private:
    uv_loop_t* loop_;
    uv_tcp_t server_;
    std::unordered_map<uv_tcp_t*, std::unique_ptr<Connection>> connections_;

    ConnectionCallback connection_callback_;
    MessageCallback message_callback_;
    ErrorCallback error_callback_;
};

} // namespace network

#endif // SERVER_H
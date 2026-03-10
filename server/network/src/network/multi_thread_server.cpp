#include "network/multi_thread_server.h"
#include "network/message.h"
#include "network/connection.h"
#include <cstring>
#include <iostream>
#include <memory>

namespace network {

MultiThreadServer::MultiThreadServer(int io_threads, int worker_threads) {
    // 创建IO线程
    for (int i = 0; i < io_threads; ++i) {
        uv_loop_t* loop = new uv_loop_t();
        uv_loop_init(loop);
        io_loops_.push_back(loop);
    }
    
    // 创建工作线程池
    worker_pool_ = std::make_unique<ThreadPool>(worker_threads);
}

MultiThreadServer::~MultiThreadServer() {
    stop();
}

bool MultiThreadServer::start(const std::string& host, int port) {
    struct sockaddr_in addr;
    uv_ip4_addr(host.c_str(), port, &addr);
    
    // 初始化主服务器
    uv_loop_t* main_loop = io_loops_[0];
    uv_tcp_init(main_loop, &server_);
    server_.data = this;
    
    int ret = uv_tcp_bind(&server_, (const struct sockaddr*)&addr, 0);
    if (ret != 0) {
        std::cerr << "Bind failed: " << uv_strerror(ret) << std::endl;
        return false;
    }
    
    ret = uv_listen((uv_stream_t*)&server_, 1024, onNewConnection);
    if (ret != 0) {
        std::cerr << "Listen failed: " << uv_strerror(ret) << std::endl;
        return false;
    }
    
    // 启动IO线程
    for (size_t i = 1; i < io_loops_.size(); ++i) {
        io_threads_.emplace_back([this, i]() {
            uv_run(io_loops_[i], UV_RUN_DEFAULT);
        });
    }
    
    std::cout << "Multi-thread server started on " << host << ":" << port << std::endl;
    
    // 运行主IO循环
    uv_run(main_loop, UV_RUN_DEFAULT);
    
    return true;
}

void MultiThreadServer::stop() {
    // 关闭服务器
    uv_close((uv_handle_t*)&server_, nullptr);
    
    // 关闭所有连接
    {
        std::lock_guard<std::mutex> lock(connections_mutex_);
        for (auto& pair : connections_) {
            uv_close((uv_handle_t*)pair.first, onClose);
        }
        connections_.clear();
    }
    
    // 停止IO线程
    for (auto loop : io_loops_) {
        uv_loop_close(loop);
        delete loop;
    }
    io_loops_.clear();
    
    // 停止工作线程池
    if (worker_pool_) {
        worker_pool_->stop();
    }
}

void MultiThreadServer::setConnectionCallback(ConnectionCallback callback) {
    connection_callback_ = callback;
}

void MultiThreadServer::setMessageCallback(MessageCallback callback) {
    message_callback_ = callback;
}

void MultiThreadServer::setErrorCallback(ErrorCallback callback) {
    error_callback_ = callback;
}

bool MultiThreadServer::send(uv_tcp_t* conn, const std::string& message) {
    std::lock_guard<std::mutex> lock(connections_mutex_);
    auto it = connections_.find(conn);
    if (it == connections_.end()) {
        return false;
    }
    
    std::string encoded_message = Message::encode(message);
    uv_buf_t buf = uv_buf_init(const_cast<char*>(encoded_message.data()), encoded_message.size());
    uv_write_t* req = new uv_write_t();
    
    int ret = uv_write(req, (uv_stream_t*)conn, &buf, 1, onWrite);
    return ret == 0;
}

void MultiThreadServer::close(uv_tcp_t* conn) {
    uv_close((uv_handle_t*)conn, onClose);
}

void MultiThreadServer::onNewConnection(uv_stream_t* server, int status) {
    if (status < 0) {
        std::cerr << "New connection error: " << uv_strerror(status) << std::endl;
        return;
    }
    
    MultiThreadServer* self = static_cast<MultiThreadServer*>(server->data);
    
    // 选择一个IO线程
    static int current_loop = 0;
    uv_loop_t* loop = self->io_loops_[current_loop];
    current_loop = (current_loop + 1) % self->io_loops_.size();
    
    uv_tcp_t* client = new uv_tcp_t();
    uv_tcp_init(loop, client);
    
    if (uv_accept(server, (uv_stream_t*)client) == 0) {
        client->data = self;
        auto connection = std::make_unique<Connection>(client);
        
        {
            std::lock_guard<std::mutex> lock(self->connections_mutex_);
            self->connections_[client] = std::move(connection);
        }
        
        // 开始读取数据
        uv_read_start((uv_stream_t*)client, onAllocBuffer, onRead);
        
        // 调用连接回调
        if (self->connection_callback_) {
            self->worker_pool_->submit([self, client]() {
                self->connection_callback_(client);
            });
        }
    } else {
        uv_close((uv_handle_t*)client, nullptr);
    }
}

void MultiThreadServer::onAllocBuffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    buf->base = new char[suggested_size];
    buf->len = suggested_size;
}

void MultiThreadServer::onRead(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
    if (nread < 0) {
        if (nread != UV_EOF) {
            std::cerr << "Read error: " << uv_strerror(nread) << std::endl;
        }
        uv_close((uv_handle_t*)stream, onClose);
    } else if (nread > 0) {
        MultiThreadServer* self = static_cast<MultiThreadServer*>(stream->data);
        uv_tcp_t* client = (uv_tcp_t*)stream;
        
        // 处理消息
        std::string data(buf->base, nread);
        size_t consumed = 0;
        std::string message = Message::decode(data, consumed);
        
        if (!message.empty()) {
            // 检查是否为心跳消息
            if (Message::isHeartbeatMessage(message)) {
                // 回复心跳
                self->send(client, Message::createHeartbeatMessage());
                // 更新心跳时间
                std::lock_guard<std::mutex> lock(self->connections_mutex_);
                auto it = self->connections_.find(client);
                if (it != self->connections_.end()) {
                    it->second->updateHeartbeat();
                }
            } else {
                // 在工作线程中处理业务逻辑
                if (self->message_callback_) {
                    self->worker_pool_->submit([self, message]() {
                        self->message_callback_(message);
                    });
                }
            }
        }
    }
    
    delete[] buf->base;
}

void MultiThreadServer::onWrite(uv_write_t* req, int status) {
    if (status < 0) {
        std::cerr << "Write error: " << uv_strerror(status) << std::endl;
    }
    delete req;
}

void MultiThreadServer::onClose(uv_handle_t* handle) {
    uv_tcp_t* client = (uv_tcp_t*)handle;
    MultiThreadServer* self = static_cast<MultiThreadServer*>(client->data);
    
    std::lock_guard<std::mutex> lock(self->connections_mutex_);
    self->connections_.erase(client);
    delete client;
}

} // namespace network
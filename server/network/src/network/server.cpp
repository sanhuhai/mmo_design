#include "network/server.h"
#include "network/message.h"
#include <cstring>
#include <iostream>
#include <memory>

namespace network {

Server::Server() : loop_(uv_default_loop()) {
    uv_tcp_init(loop_, &server_);
}

Server::~Server() {
    stop();
}

bool Server::start(const std::string& host, int port) {
    struct sockaddr_in addr;
    uv_ip4_addr(host.c_str(), port, &addr);
    
    server_.data = this;
    
    int ret = uv_tcp_bind(&server_, (const struct sockaddr*)&addr, 0);
    if (ret != 0) {
        std::cerr << "Bind failed: " << uv_strerror(ret) << std::endl;
        return false;
    }
    
    ret = uv_listen((uv_stream_t*)&server_, 1024, &Server::onNewConnection);
    if (ret != 0) {
        std::cerr << "Listen failed: " << uv_strerror(ret) << std::endl;
        return false;
    }
    
    std::cout << "Server started on " << host << ":" << port << std::endl;
    return true;
}

void Server::stop() {
    uv_close((uv_handle_t*)&server_, nullptr);
    
    // 关闭所有连接
    for (auto& pair : connections_) {
        uv_close((uv_handle_t*)pair.first, &Server::onClose);
    }
    connections_.clear();
}

void Server::setConnectionCallback(ConnectionCallback callback) {
    connection_callback_ = callback;
}

void Server::setMessageCallback(MessageCallback callback) {
    message_callback_ = callback;
}

void Server::setErrorCallback(ErrorCallback callback) {
    error_callback_ = callback;
}

bool Server::send(uv_tcp_t* conn, const std::string& message) {
    auto it = connections_.find(conn);
    if (it == connections_.end()) {
        return false;
    }
    
    std::string encoded_message = Message::encode(message);
    uv_buf_t buf = uv_buf_init(const_cast<char*>(encoded_message.data()), static_cast<unsigned int>(encoded_message.size()));
    uv_write_t* req = new uv_write_t();
    
    int ret = uv_write(req, (uv_stream_t*)conn, &buf, 1, &Server::onWrite);
    return ret == 0;
}

void Server::close(uv_tcp_t* conn) {
    uv_close((uv_handle_t*)conn, &Server::onClose);
}

void Server::onNewConnection(uv_stream_t* server, int status) {
    if (status < 0) {
        std::cerr << "New connection error: " << uv_strerror(status) << std::endl;
        return;
    }
    
    Server* self = static_cast<Server*>(server->data);
    uv_tcp_t* client = new uv_tcp_t();
    uv_tcp_init(self->loop_, client);
    
    if (uv_accept(server, (uv_stream_t*)client) == 0) {
        client->data = self;
        auto connection = std::make_unique<Connection>(client);
        self->connections_[client] = std::move(connection);
        
        // 开始读取数据
        uv_read_start((uv_stream_t*)client, &Server::onAllocBuffer, &Server::onRead);
        
        // 调用连接回调
        if (self->connection_callback_) {
            self->connection_callback_(client);
        }
    } else {
        uv_close((uv_handle_t*)client, nullptr);
    }
}

void Server::onAllocBuffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    buf->base = new char[suggested_size];
    buf->len = suggested_size;
}

void Server::onRead(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
    if (nread < 0) {
        if (nread != UV_EOF) {
            std::cerr << "Read error: " << uv_strerror(nread) << std::endl;
        }
        uv_close((uv_handle_t*)stream, &Server::onClose);
    } else if (nread > 0) {
        Server* self = static_cast<Server*>(stream->data);
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
                auto it = self->connections_.find(client);
                if (it != self->connections_.end()) {
                    it->second->updateHeartbeat();
                }
            } else {
                // 调用消息回调
                if (self->message_callback_) {
                    self->message_callback_(message);
                }
            }
        }
    }
    
    delete[] buf->base;
}

void Server::onWrite(uv_write_t* req, int status) {
    if (status < 0) {
        std::cerr << "Write error: " << uv_strerror(status) << std::endl;
    }
    delete req;
}

void Server::onClose(uv_handle_t* handle) {
    uv_tcp_t* client = (uv_tcp_t*)handle;
    Server* self = static_cast<Server*>(client->data);
    
    self->connections_.erase(client);
    delete client;
}

} // namespace network
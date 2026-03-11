#include "chat/chat_server.h"
#include <iostream>

namespace chat_service {

ChatServer* ChatServer::getInstance() {
    static ChatServer instance;
    return &instance;
}

ChatServer::ChatServer() : running_(false) {
}

ChatServer::~ChatServer() {
    stop();
    
    for (auto& pair : clients_) {
        delete pair.second;
    }
    
    for (auto& pair : channels_) {
        delete pair.second;
    }
}

bool ChatServer::initialize(const std::string& host, int port, 
                           const std::string& redis_host, int redis_port,
                           const std::string& db_host, int db_port,
                           const std::string& db_user, const std::string& db_password,
                           const std::string& db_name) {
    // 初始化世界频道
    Channel* world_channel = new WorldChannel();
    channels_[0] = world_channel;
    
    std::cout << "Chat server initialized successfully" << std::endl;
    return true;
}

void ChatServer::start() {
    running_ = true;
    std::cout << "Chat server started" << std::endl;
}

void ChatServer::stop() {
    running_ = false;
    std::cout << "Chat server stopped" << std::endl;
}

void ChatServer::handleClientConnect(int clientId) {
    std::cout << "Client connected: " << clientId << std::endl;
}

void ChatServer::handleClientDisconnect(int clientId) {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    auto it = clients_.find(clientId);
    if (it != clients_.end()) {
        delete it->second;
        clients_.erase(it);
    }
    std::cout << "Client disconnected: " << clientId << std::endl;
}

void ChatServer::handleMessage(int clientId, const ChatMessage& message) {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    auto it = clients_.find(clientId);
    if (it == clients_.end()) {
        std::cout << "Client not found: " << clientId << std::endl;
        return;
    }
    
    ClientSession* session = it->second;
    if (!session->isMessageAllowed()) {
        std::cout << "Message frequency limit exceeded for client: " << clientId << std::endl;
        return;
    }
    
    session->setLastMessageTime(std::chrono::steady_clock::now());
    
    // 过滤消息内容
    std::string filtered_content;
#ifdef PROTOBUF_FOUND
    filtered_content = message.content();
#else
    filtered_content = message.content;
#endif
    if (filter_.filterContent(filtered_content)) {
        // 内容被过滤
        std::cout << "Message filtered for client: " << clientId << std::endl;
        return;
    }
    
    // 路由消息
    router_.routeMessage(message);
}

void ChatServer::broadcastToChannel(int channelId, const ChatMessage& message) {
    std::lock_guard<std::mutex> lock(channels_mutex_);
    auto it = channels_.find(channelId);
    if (it != channels_.end()) {
        it->second->broadcast(message);
    }
}

void ChatServer::sendToUser(int userId, const ChatMessage& message) {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    for (auto& pair : clients_) {
        ClientSession* session = pair.second;
        if (session->getUserId() == userId) {
            // 发送消息给用户
            std::cout << "Sending message to user: " << userId << std::endl;
            break;
        }
    }
}

Channel* ChatServer::getChannel(int channelId) {
    std::lock_guard<std::mutex> lock(channels_mutex_);
    auto it = channels_.find(channelId);
    if (it != channels_.end()) {
        return it->second;
    }
    return nullptr;
}

ClientSession* ChatServer::getClientSession(int clientId) {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    auto it = clients_.find(clientId);
    if (it != clients_.end()) {
        return it->second;
    }
    return nullptr;
}

} // namespace chat_service

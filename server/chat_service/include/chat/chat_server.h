#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>

#include "chat/channel.h"
#include "chat/message_router.h"
#include "chat/content_filter.h"
#include "chat/client_session.h"

namespace chat_service {

class ChatServer {
public:
    static ChatServer* getInstance();
    
    bool initialize(const std::string& host, int port, 
                   const std::string& redis_host, int redis_port,
                   const std::string& db_host, int db_port,
                   const std::string& db_user, const std::string& db_password,
                   const std::string& db_name);
    
    void start();
    void stop();
    
    void handleClientConnect(int clientId);
    void handleClientDisconnect(int clientId);
    void handleMessage(int clientId, const ChatMessage& message);
    
    void broadcastToChannel(int channelId, const ChatMessage& message);
    void sendToUser(int userId, const ChatMessage& message);
    
    Channel* getChannel(int channelId);
    ClientSession* getClientSession(int clientId);
    
private:
    ChatServer();
    ~ChatServer();
    
    std::unordered_map<int, ClientSession*> clients_;
    std::unordered_map<int, Channel*> channels_;
    
    MessageRouter router_;
    ContentFilter filter_;
    
    std::mutex clients_mutex_;
    std::mutex channels_mutex_;
    
    bool running_;
};

} // namespace chat_service

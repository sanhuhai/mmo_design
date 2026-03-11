#include "chat/message_router.h"
#include "chat/chat_server.h"
#include <iostream>

namespace chat_service {

MessageRouter::MessageRouter() {
}

MessageRouter::~MessageRouter() {
}

void MessageRouter::routeMessage(const ChatMessage& message) {
#ifdef PROTOBUF_FOUND
    switch (message.type()) {
    case MessageType::WORLD:
        routeWorldMessage(message);
        break;
    case MessageType::NEARBY:
        routeNearbyMessage(message);
        break;
    case MessageType::GUILD:
        routeGuildMessage(message);
        break;
    case MessageType::PRIVATE:
        routePrivateMessage(message);
        break;
    case MessageType::STRANGER:
        routeStrangerMessage(message);
        break;
    case MessageType::SYSTEM:
        routeSystemMessage(message);
        break;
    default:
        std::cout << "Unknown message type: " << message.type() << std::endl;
        break;
    }
#else
    switch (message.type) {
    case MessageType::WORLD:
        routeWorldMessage(message);
        break;
    case MessageType::NEARBY:
        routeNearbyMessage(message);
        break;
    case MessageType::GUILD:
        routeGuildMessage(message);
        break;
    case MessageType::PRIVATE:
        routePrivateMessage(message);
        break;
    case MessageType::STRANGER:
        routeStrangerMessage(message);
        break;
    case MessageType::SYSTEM:
        routeSystemMessage(message);
        break;
    default:
        std::cout << "Unknown message type: " << message.type << std::endl;
        break;
    }
#endif
}

void MessageRouter::routeWorldMessage(const ChatMessage& message) {
    ChatServer* server = ChatServer::getInstance();
    server->broadcastToChannel(0, message); // 0 是世界频道ID
}

void MessageRouter::routeNearbyMessage(const ChatMessage& message) {
    // 实际实现中需要根据发送者位置找到附近的频道
    ChatServer* server = ChatServer::getInstance();
    server->broadcastToChannel(1, message); // 1 是附近频道ID
}

void MessageRouter::routeGuildMessage(const ChatMessage& message) {
    ChatServer* server = ChatServer::getInstance();
#ifdef PROTOBUF_FOUND
    server->broadcastToChannel(message.channel_id(), message);
#else
    server->broadcastToChannel(message.channel_id, message);
#endif
}

void MessageRouter::routePrivateMessage(const ChatMessage& message) {
    ChatServer* server = ChatServer::getInstance();
#ifdef PROTOBUF_FOUND
    server->sendToUser(message.target_id(), message);
#else
    server->sendToUser(message.target_id, message);
#endif
}

void MessageRouter::routeStrangerMessage(const ChatMessage& message) {
    ChatServer* server = ChatServer::getInstance();
#ifdef PROTOBUF_FOUND
    server->sendToUser(message.target_id(), message);
#else
    server->sendToUser(message.target_id, message);
#endif
}

void MessageRouter::routeSystemMessage(const ChatMessage& message) {
    ChatServer* server = ChatServer::getInstance();
    server->broadcastToChannel(0, message); // 系统消息广播到世界频道
}

} // namespace chat_service

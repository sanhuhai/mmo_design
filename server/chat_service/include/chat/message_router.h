#pragma once

#include "chat/chat_message.h"

namespace chat_service {

class MessageRouter {
public:
    MessageRouter();
    ~MessageRouter();
    
    void routeMessage(const ChatMessage& message);
    
private:
    void routeWorldMessage(const ChatMessage& message);
    void routeNearbyMessage(const ChatMessage& message);
    void routeGuildMessage(const ChatMessage& message);
    void routePrivateMessage(const ChatMessage& message);
    void routeStrangerMessage(const ChatMessage& message);
    void routeSystemMessage(const ChatMessage& message);
};

} // namespace chat_service

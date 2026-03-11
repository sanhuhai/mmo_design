#pragma once

#include <string>
#include <chrono>

namespace chat_service {

class ClientSession {
public:
    ClientSession(int clientId, int userId, const std::string& username);
    ~ClientSession();
    
    int getClientId() const;
    int getUserId() const;
    const std::string& getUsername() const;
    
    void setLastMessageTime(const std::chrono::steady_clock::time_point& time);
    std::chrono::steady_clock::time_point getLastMessageTime() const;
    
    bool isMessageAllowed();
    
    void setPosition(float x, float y, float z);
    void getPosition(float& x, float& y, float& z) const;
    
private:
    int client_id_;
    int user_id_;
    std::string username_;
    std::chrono::steady_clock::time_point last_message_time_;
    float x_;
    float y_;
    float z_;
    
    static constexpr int MESSAGE_INTERVAL_MS = 1000; // 消息发送间隔限制
};

} // namespace chat_service

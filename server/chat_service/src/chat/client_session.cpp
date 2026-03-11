#include "chat/client_session.h"
#include <chrono>

namespace chat_service {

ClientSession::ClientSession(int clientId, int userId, const std::string& username)
    : client_id_(clientId), user_id_(userId), username_(username),
      last_message_time_(std::chrono::steady_clock::now()),
      x_(0.0f), y_(0.0f), z_(0.0f) {
}

ClientSession::~ClientSession() {
}

int ClientSession::getClientId() const {
    return client_id_;
}

int ClientSession::getUserId() const {
    return user_id_;
}

const std::string& ClientSession::getUsername() const {
    return username_;
}

void ClientSession::setLastMessageTime(const std::chrono::steady_clock::time_point& time) {
    last_message_time_ = time;
}

std::chrono::steady_clock::time_point ClientSession::getLastMessageTime() const {
    return last_message_time_;
}

bool ClientSession::isMessageAllowed() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_message_time_).count();
    return elapsed >= MESSAGE_INTERVAL_MS;
}

void ClientSession::setPosition(float x, float y, float z) {
    x_ = x;
    y_ = y;
    z_ = z;
}

void ClientSession::getPosition(float& x, float& y, float& z) const {
    x = x_;
    y = y_;
    z = z_;
}

} // namespace chat_service

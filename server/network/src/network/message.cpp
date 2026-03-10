#include "network/message.h"
#include <cstring>

namespace network {

std::string Message::encode(const std::string& message) {
    // 简单的消息编码：前缀长度
    uint32_t length = message.size();
    std::string encoded;
    encoded.append(reinterpret_cast<const char*>(&length), sizeof(length));
    encoded.append(message);
    return encoded;
}

std::string Message::decode(const std::string& data, size_t& consumed) {
    // 简单的消息解码：读取长度前缀
    if (data.size() < sizeof(uint32_t)) {
        consumed = 0;
        return "";
    }
    
    uint32_t length = *reinterpret_cast<const uint32_t*>(data.data());
    if (data.size() < sizeof(uint32_t) + length) {
        consumed = 0;
        return "";
    }
    
    consumed = sizeof(uint32_t) + length;
    return data.substr(sizeof(uint32_t), length);
}

std::string Message::createHeartbeatMessage() {
    return "__heartbeat__";
}

bool Message::isHeartbeatMessage(const std::string& message) {
    return message == "__heartbeat__";
}

} // namespace network
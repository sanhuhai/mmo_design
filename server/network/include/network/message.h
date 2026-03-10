#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>

namespace network {

class Message {
public:
    static std::string encode(const std::string& message);
    static std::string decode(const std::string& data, size_t& consumed);
    static std::string createHeartbeatMessage();
    static bool isHeartbeatMessage(const std::string& message);
};

} // namespace network

#endif // MESSAGE_H
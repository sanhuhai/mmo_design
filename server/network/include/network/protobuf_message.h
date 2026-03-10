#ifndef PROTOBUF_MESSAGE_H
#define PROTOBUF_MESSAGE_H

#include <string>
#include "game_message.pb.h"

namespace network {

class ProtobufMessage {
public:
    // 序列化消息
    static std::string serialize(const game::GameMessage& message);

    // 反序列化消息
    static game::GameMessage deserialize(const std::string& data);

    // 创建心跳消息
    static game::GameMessage createHeartbeatMessage();

    // 检查是否为心跳消息
    static bool isHeartbeatMessage(const game::GameMessage& message);
};

} // namespace network

#endif // PROTOBUF_MESSAGE_H
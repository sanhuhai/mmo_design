#include "network/protobuf_message.h"

namespace network {

std::string ProtobufMessage::serialize(const game::GameMessage& message) {
    std::string data;
    message.SerializeToString(&data);
    return data;
}

game::GameMessage ProtobufMessage::deserialize(const std::string& data) {
    game::GameMessage message;
    message.ParseFromString(data);
    return message;
}

game::GameMessage ProtobufMessage::createHeartbeatMessage() {
    game::GameMessage message;
    message.set_type(game::MessageType::HEARTBEAT);
    auto* heartbeat = message.mutable_heartbeat();
    heartbeat->set_timestamp(time(nullptr));
    return message;
}

bool ProtobufMessage::isHeartbeatMessage(const game::GameMessage& message) {
    return message.type() == game::MessageType::HEARTBEAT;
}

} // namespace network
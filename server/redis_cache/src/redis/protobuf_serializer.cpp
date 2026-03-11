#include "redis/protobuf_serializer.h"
#include "player_data.pb.h"

namespace redis {

std::string ProtobufSerializer::serializePlayerData(const game::PlayerData& data) {
    return serialize(data);
}

bool ProtobufSerializer::deserializePlayerData(const std::string& data, game::PlayerData& result) {
    return deserialize(data, result);
}

std::string ProtobufSerializer::serializePlayerBaseInfo(const game::PlayerBaseInfo& info) {
    return serialize(info);
}

bool ProtobufSerializer::deserializePlayerBaseInfo(const std::string& data, game::PlayerBaseInfo& result) {
    return deserialize(data, result);
}

std::string ProtobufSerializer::serializePlayerInventory(const game::PlayerInventoryInfo& info) {
    return serialize(info);
}

bool ProtobufSerializer::deserializePlayerInventory(const std::string& data, game::PlayerInventoryInfo& result) {
    return deserialize(data, result);
}

} // namespace redis

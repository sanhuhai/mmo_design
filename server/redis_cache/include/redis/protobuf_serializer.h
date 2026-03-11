#pragma once

#include <string>
#include <memory>

// 包含 protobuf 生成的头文件
#include "player_data.pb.h"

namespace redis {

class ProtobufSerializer {
public:
    ProtobufSerializer() = default;
    ~ProtobufSerializer() = default;
    
    template<typename T>
    std::string serialize(const T& message) {
        std::string data;
        if (!message.SerializeToString(&data)) {
            return "";
        }
        return data;
    }
    
    template<typename T>
    bool deserialize(const std::string& data, T& message) {
        return message.ParseFromString(data);
    }
    
    std::string serializePlayerData(const game::PlayerData& data);
    bool deserializePlayerData(const std::string& data, game::PlayerData& result);
    
    std::string serializePlayerBaseInfo(const game::PlayerBaseInfo& info);
    bool deserializePlayerBaseInfo(const std::string& data, game::PlayerBaseInfo& result);
    
    std::string serializePlayerInventory(const game::PlayerInventoryInfo& info);
    bool deserializePlayerInventory(const std::string& data, game::PlayerInventoryInfo& result);
};

} // namespace redis

#pragma once

#include <string>
#include <vector>

#ifdef PROTOBUF_FOUND
#include "chat.pb.h"
#endif

namespace chat_service {

#ifdef PROTOBUF_FOUND
using ChatMessage = chat_service::ChatMessage;
using MessageType = chat_service::MessageType;
using MessageResponse = chat_service::MessageResponse;
using ChannelMembersRequest = chat_service::ChannelMembersRequest;
using ChannelMembersResponse = chat_service::ChannelMembersResponse;
using MessageHistoryRequest = chat_service::MessageHistoryRequest;
using MessageHistoryResponse = chat_service::MessageHistoryResponse;
#else
// 非Protobuf版本的消息结构
enum MessageType {
    WORLD = 0,      // 世界聊天
    NEARBY = 1,     // 附近聊天
    GUILD = 2,      // 公会聊天
    PRIVATE = 3,    // 好友私聊
    STRANGER = 4,   // 陌生人聊天
    SYSTEM = 5,     // 系统消息
};

struct ChatMessage {
    int message_id;
    int sender_id;
    std::string sender_name;
    MessageType type;
    std::string content;
    int64_t timestamp;
    int channel_id;
    int target_id;
    float sender_x;
    float sender_y;
    float sender_z;
};

struct MessageResponse {
    int code;
    std::string message;
    int message_id;
    int64_t timestamp;
};

struct ChannelMembersRequest {
    int channel_id;
    MessageType channel_type;
};

struct ChannelMembersResponse {
    int code;
    std::string message;
    std::vector<int> member_ids;
    std::vector<std::string> member_names;
};

struct MessageHistoryRequest {
    MessageType message_type;
    int channel_id;
    int64_t start_time;
    int64_t end_time;
    int limit;
};

struct MessageHistoryResponse {
    int code;
    std::string message;
    std::vector<ChatMessage> messages;
};
#endif

} // namespace chat_service

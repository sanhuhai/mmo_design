#include "chat/channel.h"
#include <iostream>
#include <cmath>

namespace chat_service {

Channel::Channel(int channelId, const std::string& name) 
    : channel_id_(channelId), name_(name) {
}

Channel::~Channel() {
}

void Channel::addMember(int userId) {
    std::lock_guard<std::mutex> lock(members_mutex_);
    members_.insert(userId);
}

void Channel::removeMember(int userId) {
    std::lock_guard<std::mutex> lock(members_mutex_);
    members_.erase(userId);
}

std::vector<int> Channel::getMembers() {
    std::lock_guard<std::mutex> lock(members_mutex_);
    return std::vector<int>(members_.begin(), members_.end());
}

int Channel::getChannelId() const {
    return channel_id_;
}

const std::string& Channel::getName() const {
    return name_;
}

WorldChannel::WorldChannel() : Channel(0, "World Channel") {
}

WorldChannel::~WorldChannel() {
}

void WorldChannel::broadcast(const ChatMessage& message) {
#ifdef PROTOBUF_FOUND
    std::cout << "World channel broadcast: " << message.content() << std::endl;
#else
    std::cout << "World channel broadcast: " << message.content << std::endl;
#endif
    // 实际实现中需要将消息发送给所有在线玩家
}

NearbyChannel::NearbyChannel(int channelId, float x, float y, float z, float radius)
    : Channel(channelId, "Nearby Channel"),
      center_x_(x), center_y_(y), center_z_(z), radius_(radius) {
}

NearbyChannel::~NearbyChannel() {
}

void NearbyChannel::broadcast(const ChatMessage& message) {
#ifdef PROTOBUF_FOUND
    std::cout << "Nearby channel broadcast: " << message.content() << std::endl;
#else
    std::cout << "Nearby channel broadcast: " << message.content << std::endl;
#endif
    // 实际实现中需要将消息发送给附近的玩家
}

void NearbyChannel::addMember(int userId) {
    Channel::addMember(userId);
    // 记录成员位置
}

void NearbyChannel::removeMember(int userId) {
    Channel::removeMember(userId);
    std::lock_guard<std::mutex> lock(positions_mutex_);
    member_positions_.erase(userId);
}

void NearbyChannel::updateMemberPosition(int userId, float x, float y, float z) {
    std::lock_guard<std::mutex> lock(positions_mutex_);
    member_positions_[userId] = std::make_tuple(x, y, z);
}

bool NearbyChannel::isInRange(float x, float y, float z) const {
    float dx = x - center_x_;
    float dy = y - center_y_;
    float dz = z - center_z_;
    float distance = std::sqrt(dx*dx + dy*dy + dz*dz);
    return distance <= radius_;
}

GuildChannel::GuildChannel(int guildId, const std::string& guildName)
    : Channel(guildId, guildName) {
}

GuildChannel::~GuildChannel() {
}

void GuildChannel::broadcast(const ChatMessage& message) {
#ifdef PROTOBUF_FOUND
    std::cout << "Guild channel broadcast: " << message.content() << std::endl;
#else
    std::cout << "Guild channel broadcast: " << message.content << std::endl;
#endif
    // 实际实现中需要将消息发送给公会成员
}

PrivateChannel::PrivateChannel(int userId1, int userId2)
    : Channel(userId1 + userId2, "Private Channel") {
    addMember(userId1);
    addMember(userId2);
}

PrivateChannel::~PrivateChannel() {
}

void PrivateChannel::broadcast(const ChatMessage& message) {
#ifdef PROTOBUF_FOUND
    std::cout << "Private channel broadcast: " << message.content() << std::endl;
#else
    std::cout << "Private channel broadcast: " << message.content << std::endl;
#endif
    // 实际实现中需要将消息发送给私聊对象
}

void PrivateChannel::addMember(int userId) {
    // 私聊频道只能有两个成员
    std::lock_guard<std::mutex> lock(members_mutex_);
    if (members_.size() < 2) {
        members_.insert(userId);
    }
}

void PrivateChannel::removeMember(int userId) {
    Channel::removeMember(userId);
    // 如果频道为空，删除频道
}

} // namespace chat_service

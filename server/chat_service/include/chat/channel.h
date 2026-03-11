#pragma once

#include "chat/chat_message.h"

#include <vector>
#include <string>
#include <unordered_set>
#include <mutex>

namespace chat_service {

class Channel {
public:
    Channel(int channelId, const std::string& name);
    virtual ~Channel();
    
    virtual void broadcast(const ChatMessage& message) = 0;
    virtual void addMember(int userId);
    virtual void removeMember(int userId);
    virtual std::vector<int> getMembers();
    
    int getChannelId() const;
    const std::string& getName() const;
    
protected:
    int channel_id_;
    std::string name_;
    std::unordered_set<int> members_;
    std::mutex members_mutex_;
};

class WorldChannel : public Channel {
public:
    WorldChannel();
    ~WorldChannel() override;
    
    void broadcast(const ChatMessage& message) override;
};

class NearbyChannel : public Channel {
public:
    NearbyChannel(int channelId, float x, float y, float z, float radius);
    ~NearbyChannel() override;
    
    void broadcast(const ChatMessage& message) override;
    void addMember(int userId) override;
    void removeMember(int userId) override;
    
    void updateMemberPosition(int userId, float x, float y, float z);
    bool isInRange(float x, float y, float z) const;
    
private:
    float center_x_;
    float center_y_;
    float center_z_;
    float radius_;
    std::unordered_map<int, std::tuple<float, float, float>> member_positions_;
    std::mutex positions_mutex_;
};

class GuildChannel : public Channel {
public:
    GuildChannel(int guildId, const std::string& guildName);
    ~GuildChannel() override;
    
    void broadcast(const ChatMessage& message) override;
};

class PrivateChannel : public Channel {
public:
    PrivateChannel(int userId1, int userId2);
    ~PrivateChannel() override;
    
    void broadcast(const ChatMessage& message) override;
    void addMember(int userId) override;
    void removeMember(int userId) override;
};

} // namespace chat_service

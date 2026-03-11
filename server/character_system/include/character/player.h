#ifndef PLAYER_H
#define PLAYER_H

#include "character/character.h"
#include <string>
#include <memory>
#include <vector>
#include <map>

namespace game {
namespace character {

class Player : public Character {
public:
    Player(uint64_t id, const std::string& name, uint64_t account_id);
    ~Player() override = default;

    uint64_t getAccountId() const { return account_id_; }
    void setAccountId(uint64_t account_id) { account_id_ = account_id; }

    const std::string& getNickname() const { return nickname_; }
    void setNickname(const std::string& nickname) { nickname_ = nickname; }

    int32_t getLevel() const { return level_; }
    void setLevel(int32_t level) { level_ = level; }

    int64_t getExperience() const { return experience_; }
    void setExperience(int64_t experience) { experience_ = experience; }

    int64_t getMaxExperience() const { return max_experience_; }
    void setMaxExperience(int64_t max_experience) { max_experience_ = max_experience; }

    void addExperience(int64_t exp);
    bool checkLevelUp();

    int32_t getGold() const { return gold_; }
    void setGold(int32_t gold) { gold_ = gold; }
    void addGold(int32_t amount) { gold_ += amount; }
    bool spendGold(int32_t amount);

    int32_t getDiamond() const { return diamond_; }
    void setDiamond(int32_t diamond) { diamond_ = diamond; }
    void addDiamond(int32_t amount) { diamond_ += amount; }
    bool spendDiamond(int32_t amount);

    void update(float delta_time) override;
    void onDeath() override;

    void saveData();
    void loadData();

    void setOnline(bool online) { is_online_ = online; }
    bool isOnline() const { return is_online_; }

    void setLastLoginTime(uint64_t timestamp) { last_login_time_ = timestamp; }
    uint64_t getLastLoginTime() const { return last_login_time_; }

    void setLastLogoutTime(uint64_t timestamp) { last_logout_time_ = timestamp; }
    uint64_t getLastLogoutTime() const { return last_logout_time_; }

private:
    uint64_t account_id_;
    std::string nickname_;
    
    int32_t level_;
    int64_t experience_;
    int64_t max_experience_;
    
    int32_t gold_;
    int32_t diamond_;
    
    bool is_online_;
    uint64_t last_login_time_;
    uint64_t last_logout_time_;
};

} 
} 

#endif 

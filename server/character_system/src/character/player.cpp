#include "character/player.h"
#include "character/combat_component.h"
#include <iostream>

namespace game {
namespace character {

Player::Player(uint64_t id, const std::string& name, uint64_t account_id)
    : Character(id, name, CharacterType::PLAYER)
    , account_id_(account_id)
    , nickname_(name)
    , level_(1)
    , experience_(0)
    , max_experience_(100)
    , gold_(0)
    , diamond_(0)
    , is_online_(false)
    , last_login_time_(0)
    , last_logout_time_(0) {
}

void Player::update(float delta_time) {
    Character::update(delta_time);
}

void Player::onDeath() {
    Character::onDeath();
    
    std::cout << "Player " << name_ << " has died!" << std::endl;
}

void Player::addExperience(int64_t exp) {
    experience_ += exp;
    
    if (experience_ >= max_experience_) {
        checkLevelUp();
    }
}

bool Player::checkLevelUp() {
    if (experience_ < max_experience_) {
        return false;
    }
    
    level_++;
    experience_ -= max_experience_;
    max_experience_ = static_cast<int64_t>(max_experience_ * 1.2f);
    
    auto combat = getComponent<CombatComponent>();
    if (combat) {
        auto attrs = combat->getAttributes();
        attrs.max_health += 10.0f;
        attrs.health = attrs.max_health;
        attrs.max_mana += 5.0f;
        attrs.mana = attrs.max_mana;
        attrs.attack += 2.0f;
        attrs.defense += 1.0f;
        combat->setAttributes(attrs);
    }
    
    std::cout << "Player " << name_ << " leveled up to level " << level_ << "!" << std::endl;
    
    return true;
}

bool Player::spendGold(int32_t amount) {
    if (gold_ < amount) {
        return false;
    }
    
    gold_ -= amount;
    return true;
}

bool Player::spendDiamond(int32_t amount) {
    if (diamond_ < amount) {
        return false;
    }
    
    diamond_ -= amount;
    return true;
}

void Player::saveData() {
    std::cout << "Saving player data for " << name_ << std::endl;
}

void Player::loadData() {
    std::cout << "Loading player data for " << name_ << std::endl;
}

} 
} 

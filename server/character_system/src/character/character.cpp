#include "character/character.h"
#include "character/component.h"
#include <iostream>

namespace game {
namespace character {

Character::Character(uint64_t id, const std::string& name, CharacterType type)
    : id_(id), name_(name), type_(type), is_alive_(true) {
}

Character::~Character() {
    components_.clear();
}

void Character::update(float delta_time) {
    if (!is_alive_) {
        return;
    }

    for (auto& pair : components_) {
        pair.second->update(delta_time);
    }
}

void Character::onDeath() {
    is_alive_ = false;
}

} 
} 

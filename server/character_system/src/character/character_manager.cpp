#include "character/character_manager.h"
#include "character/combat_component.h"
#include "character/movement_component.h"
#include <algorithm>
#include <iostream>

namespace game {
namespace character {

CharacterManager& CharacterManager::getInstance() {
    static CharacterManager instance;
    return instance;
}

void CharacterManager::initialize() {
    std::cout << "CharacterManager initialized" << std::endl;
}

void CharacterManager::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    removeAllCharacters();
    std::cout << "CharacterManager shutdown" << std::endl;
}

void CharacterManager::update(float delta_time) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (auto character : characters_) {
        if (character) {
            character->update(delta_time);
        }
    }
}

Character* CharacterManager::createCharacter(uint64_t id, const std::string& name, CharacterType type) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (characters_map_.find(id) != characters_map_.end()) {
        std::cerr << "Character with id " << id << " already exists" << std::endl;
        return nullptr;
    }
    
    std::unique_ptr<Character> character = std::make_unique<Character>(id, name, type);
    Character* ptr = character.get();
    
    characters_map_[id] = std::move(character);
    characters_.push_back(ptr);
    
    if (on_character_created_callback_) {
        on_character_created_callback_(ptr);
    }
    
    std::cout << "Created character: " << name << " (ID: " << id << ")" << std::endl;
    
    return ptr;
}

Player* CharacterManager::createPlayer(uint64_t id, const std::string& name, uint64_t account_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (characters_map_.find(id) != characters_map_.end()) {
        std::cerr << "Player with id " << id << " already exists" << std::endl;
        return nullptr;
    }
    
    std::unique_ptr<Player> player = std::make_unique<Player>(id, name, account_id);
    Player* ptr = player.get();
    
    characters_map_[id] = std::move(player);
    characters_.push_back(ptr);
    players_.push_back(ptr);
    
    if (on_character_created_callback_) {
        on_character_created_callback_(ptr);
    }
    
    std::cout << "Created player: " << name << " (ID: " << id << ", Account: " << account_id << ")" << std::endl;
    
    return ptr;
}

NPC* CharacterManager::createNPC(uint64_t id, const std::string& name, NPCType npc_type) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (characters_map_.find(id) != characters_map_.end()) {
        std::cerr << "NPC with id " << id << " already exists" << std::endl;
        return nullptr;
    }
    
    std::unique_ptr<NPC> npc = std::make_unique<NPC>(id, name, npc_type);
    NPC* ptr = npc.get();
    
    characters_map_[id] = std::move(npc);
    characters_.push_back(ptr);
    npcs_.push_back(ptr);
    
    if (on_character_created_callback_) {
        on_character_created_callback_(ptr);
    }
    
    std::cout << "Created NPC: " << name << " (ID: " << id << ")" << std::endl;
    
    return ptr;
}

void CharacterManager::removeCharacter(uint64_t id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = characters_map_.find(id);
    if (it == characters_map_.end()) {
        std::cerr << "Character with id " << id << " not found" << std::endl;
        return;
    }
    
    Character* character = it->second.get();
    
    if (on_character_removed_callback_) {
        on_character_removed_callback_(character);
    }
    
    characters_.erase(std::remove(characters_.begin(), characters_.end(), character), characters_.end());
    
    if (character->getType() == CharacterType::PLAYER) {
        Player* player = static_cast<Player*>(character);
        players_.erase(std::remove(players_.begin(), players_.end(), player), players_.end());
    } else if (character->getType() == CharacterType::NPC) {
        NPC* npc = static_cast<NPC*>(character);
        npcs_.erase(std::remove(npcs_.begin(), npcs_.end(), npc), npcs_.end());
    }
    
    characters_map_.erase(it);
    
    std::cout << "Removed character: " << character->getName() << " (ID: " << id << ")" << std::endl;
}

void CharacterManager::removeAllCharacters() {
    for (auto character : characters_) {
        if (on_character_removed_callback_) {
            on_character_removed_callback_(character);
        }
    }
    
    characters_.clear();
    players_.clear();
    npcs_.clear();
    characters_map_.clear();
    
    std::cout << "Removed all characters" << std::endl;
}

Character* CharacterManager::getCharacter(uint64_t id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = characters_map_.find(id);
    if (it != characters_map_.end()) {
        return it->second.get();
    }
    
    return nullptr;
}

Player* CharacterManager::getPlayer(uint64_t id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = characters_map_.find(id);
    if (it != characters_map_.end()) {
        Character* character = it->second.get();
        if (character->getType() == CharacterType::PLAYER) {
            return static_cast<Player*>(character);
        }
    }
    
    return nullptr;
}

NPC* CharacterManager::getNPC(uint64_t id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = characters_map_.find(id);
    if (it != characters_map_.end()) {
        Character* character = it->second.get();
        if (character->getType() == CharacterType::NPC) {
            return static_cast<NPC*>(character);
        }
    }
    
    return nullptr;
}

std::vector<Character*> CharacterManager::getCharactersInRange(const Vector3& position, float range) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<Character*> result;
    
    for (auto character : characters_) {
        auto movement = character->getComponent<MovementComponent>();
        if (movement) {
            Vector3 char_position = movement->getPosition();
            float distance = (char_position - position).length();
            if (distance <= range) {
                result.push_back(character);
            }
        }
    }
    
    return result;
}

std::vector<Player*> CharacterManager::getPlayersInRange(const Vector3& position, float range) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<Player*> result;
    
    for (auto player : players_) {
        auto movement = player->getComponent<MovementComponent>();
        if (movement) {
            Vector3 char_position = movement->getPosition();
            float distance = (char_position - position).length();
            if (distance <= range) {
                result.push_back(player);
            }
        }
    }
    
    return result;
}

std::vector<NPC*> CharacterManager::getNPCsInRange(const Vector3& position, float range) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<NPC*> result;
    
    for (auto npc : npcs_) {
        auto movement = npc->getComponent<MovementComponent>();
        if (movement) {
            Vector3 char_position = movement->getPosition();
            float distance = (char_position - position).length();
            if (distance <= range) {
                result.push_back(npc);
            }
        }
    }
    
    return result;
}

CharacterManager::CharacterManager() {
}

} 
} 

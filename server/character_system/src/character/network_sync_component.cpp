#include "character/network_sync_component.h"
#include "character/character.h"
#include "character/movement_component.h"
#include "character/combat_component.h"
#include <chrono>
#include <cstring>

namespace game {
namespace character {

NetworkSyncComponent::NetworkSyncComponent(Character* owner)
    : Component(owner)
    , sync_interval_(0.1f)
    , sync_timer_(0.0f)
    , sync_priority_(SyncPriority::MEDIUM)
    , is_dirty_(false)
    , position_dirty_(false)
    , state_dirty_(false)
    , attributes_dirty_(false)
    , enable_prediction_(false)
    , enable_rollback_(false) {
    
    last_sync_data_.character_id = owner->getId();
    last_sync_data_.timestamp = 0;
}

void NetworkSyncComponent::initialize() {
    Component::initialize();
    is_dirty_ = false;
    sync_timer_ = 0.0f;
}

void NetworkSyncComponent::update(float delta_time) {
    Component::update(delta_time);
    
    updateSyncTimer(delta_time);
    
    if (shouldSync()) {
        performSync();
    }
}

void NetworkSyncComponent::updateSyncTimer(float delta_time) {
    sync_timer_ += delta_time;
}

bool NetworkSyncComponent::shouldSync() const {
    if (!is_dirty_) {
        return false;
    }
    
    if (sync_priority_ == SyncPriority::CRITICAL) {
        return true;
    }
    
    if (sync_timer_ >= sync_interval_) {
        return true;
    }
    
    return false;
}

void NetworkSyncComponent::performSync() {
    SyncData data = getSyncData();
    
    if (on_sync_callback_) {
        on_sync_callback_(data);
    }
    
    last_sync_data_ = data;
    is_dirty_ = false;
    position_dirty_ = false;
    state_dirty_ = false;
    attributes_dirty_ = false;
    sync_timer_ = 0.0f;
    
    if (enable_prediction_) {
        prediction_history_.push_back(data);
        if (prediction_history_.size() > 10) {
            prediction_history_.erase(prediction_history_.begin());
        }
    }
}

SyncData NetworkSyncComponent::getSyncData() const {
    SyncData data;
    data.character_id = owner_->getId();
    
    auto movement = owner_->getComponent<MovementComponent>();
    if (movement) {
        data.position = movement->getPosition();
    }
    
    auto combat = owner_->getComponent<CombatComponent>();
    if (combat) {
        data.state = combat->getCurrentState();
        data.health = combat->getAttributes().health;
        data.max_health = combat->getAttributes().max_health;
        data.mana = combat->getAttributes().mana;
        data.max_mana = combat->getAttributes().max_mana;
    }
    
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    data.timestamp = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
    
    return data;
}

void NetworkSyncComponent::applySyncData(const SyncData& data) {
    if (data.character_id != owner_->getId()) {
        return;
    }
    
    if (enable_rollback_ && enable_prediction_) {
        for (auto it = prediction_history_.rbegin(); it != prediction_history_.rend(); ++it) {
            if (it->timestamp == data.timestamp) {
                return;
            }
        }
        
        for (auto it = prediction_history_.rbegin(); it != prediction_history_.rend(); ++it) {
            if (it->timestamp < data.timestamp) {
                break;
            }
            
            auto movement = owner_->getComponent<MovementComponent>();
            if (movement && position_dirty_) {
                movement->setPosition(it->position);
            }
        }
    }
    
    auto movement = owner_->getComponent<MovementComponent>();
    if (movement && position_dirty_) {
        movement->setPosition(data.position);
    }
    
    auto combat = owner_->getComponent<CombatComponent>();
    if (combat) {
        if (state_dirty_) {
            combat->setState(data.state);
        }
        
        if (attributes_dirty_) {
            auto attrs = combat->getAttributes();
            attrs.health = data.health;
            attrs.max_health = data.max_health;
            attrs.mana = data.mana;
            attrs.max_mana = data.max_mana;
            combat->setAttributes(attrs);
        }
    }
}

void NetworkSyncComponent::sendRPC(const std::string& method_name, const std::vector<uint8_t>& parameters, uint64_t target_id) {
    RPCRequest request;
    request.rpc_id = static_cast<uint32_t>(rand());
    request.method_name = method_name;
    request.parameters = parameters;
    request.target_id = target_id;
    
    if (on_rpc_callback_) {
        on_rpc_callback_(request);
    }
}

void NetworkSyncComponent::handleRPC(const RPCRequest& request) {
    if (request.target_id != 0 && request.target_id != owner_->getId()) {
        return;
    }
    
    if (request.method_name == "moveTo") {
        if (request.parameters.size() >= 12) {
            Vector3 position;
            std::memcpy(&position.x, &request.parameters[0], sizeof(float));
            std::memcpy(&position.y, &request.parameters[4], sizeof(float));
            std::memcpy(&position.z, &request.parameters[8], sizeof(float));
            
            auto movement = owner_->getComponent<MovementComponent>();
            if (movement) {
                movement->moveTo(position);
            }
        }
    } else if (request.method_name == "attack") {
        if (request.parameters.size() >= 8) {
            uint64_t target_id;
            std::memcpy(&target_id, &request.parameters[0], sizeof(uint64_t));
            
            auto combat = owner_->getComponent<CombatComponent>();
            if (combat) {
            }
        }
    } else if (request.method_name == "castSkill") {
        if (request.parameters.size() >= 12) {
            uint32_t skill_id;
            uint64_t target_id;
            std::memcpy(&skill_id, &request.parameters[0], sizeof(uint32_t));
            std::memcpy(&target_id, &request.parameters[4], sizeof(uint64_t));
            
            auto combat = owner_->getComponent<CombatComponent>();
            if (combat) {
            }
        }
    }
}

} 
} 

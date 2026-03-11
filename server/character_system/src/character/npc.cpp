#include "character/npc.h"
#include "character/combat_component.h"
#include "character/movement_component.h"
#include "character/character_types.h"
#include <cmath>
#include <iostream>
#include <random>

namespace game {
namespace character {

NPC::NPC(uint64_t id, const std::string& name, NPCType type)
    : Character(id, name, CharacterType::NPC)
    , npc_type_(type)
    , ai_state_(AIState::IDLE)
    , target_id_(0)
    , aggro_range_(10.0f)
    , attack_range_(3.0f)
    , patrol_radius_(20.0f)
    , patrol_timer_(0.0f)
    , respawn_delay_(30.0f)
    , respawn_timer_(0.0f) {
    
    spawn_position_ = Vector3(0.0f, 0.0f, 0.0f);
    patrol_target_ = spawn_position_;
}

void NPC::update(float delta_time) {
    if (!isAlive()) {
        if (respawn_timer_ > 0.0f) {
            respawn_timer_ -= delta_time;
            if (respawn_timer_ <= 0.0f) {
                respawn(respawn_delay_);
            }
        }
        return;
    }
    
    Character::update(delta_time);
    updateAI(delta_time);
}

void NPC::updateAI(float delta_time) {
    switch (ai_state_) {
        case AIState::IDLE:
            if (checkAggro()) {
                setAIState(AIState::CHASE);
            } else {
                updatePatrol(delta_time);
            }
            break;
            
        case AIState::PATROL:
            if (checkAggro()) {
                setAIState(AIState::CHASE);
            } else {
                updatePatrol(delta_time);
            }
            break;
            
        case AIState::CHASE:
            if (checkDeaggro()) {
                setAIState(AIState::IDLE);
                target_id_ = 0;
            } else {
                updateChase(delta_time);
            }
            break;
            
        case AIState::ATTACK:
            if (checkDeaggro()) {
                setAIState(AIState::IDLE);
                target_id_ = 0;
            } else {
                updateAttack(delta_time);
            }
            break;
            
        case AIState::FLEE:
            updateFlee(delta_time);
            break;
            
        case AIState::DEAD:
            break;
    }
}

void NPC::updatePatrol(float delta_time) {
    auto movement = getComponent<MovementComponent>();
    if (!movement) {
        return;
    }
    
    if (!movement->isMoving()) {
        patrol_timer_ -= delta_time;
        if (patrol_timer_ <= 0.0f) {
            patrol_timer_ = 5.0f + static_cast<float>(rand()) / RAND_MAX * 5.0f;
            
            float angle = static_cast<float>(rand()) / RAND_MAX * 6.28318f;
            float distance = static_cast<float>(rand()) / RAND_MAX * patrol_radius_;
            
            patrol_target_.x = spawn_position_.x + std::cos(angle) * distance;
            patrol_target_.y = spawn_position_.y;
            patrol_target_.z = spawn_position_.z + std::sin(angle) * distance;
            
            movement->moveTo(patrol_target_);
        }
    }
}

void NPC::updateChase(float delta_time) {
    auto movement = getComponent<MovementComponent>();
    auto combat = getComponent<CombatComponent>();
    
    if (!movement || !combat) {
        return;
    }
    
    if (target_id_ == 0) {
        setAIState(AIState::IDLE);
        return;
    }
    
    Vector3 target_position(0.0f, 0.0f, 0.0f);
    
    float distance = (target_position - movement->getPosition()).length();
    
    if (distance <= attack_range_) {
        setAIState(AIState::ATTACK);
    } else {
        movement->moveTo(target_position);
    }
}

void NPC::updateAttack(float delta_time) {
    auto combat = getComponent<CombatComponent>();
    if (!combat) {
        return;
    }
    
    if (combat->canAttack()) {
    }
}

void NPC::updateFlee(float delta_time) {
    auto movement = getComponent<MovementComponent>();
    if (!movement) {
        return;
    }
    
    Vector3 flee_direction = movement->getPosition() - spawn_position_;
    flee_direction.normalize();
    
    Vector3 flee_target = movement->getPosition() + flee_direction * 20.0f;
    movement->moveTo(flee_target);
    
    float distance = (movement->getPosition() - spawn_position_).length();
    if (distance > patrol_radius_ * 2.0f) {
        setAIState(AIState::IDLE);
        movement->stopMovement();
    }
}

bool NPC::checkAggro() {
    auto movement = getComponent<MovementComponent>();
    if (!movement) {
        return false;
    }
    
    Vector3 position = movement->getPosition();
    
    float min_distance = aggro_range_;
    uint64_t closest_target = 0;
    
    if (min_distance < aggro_range_) {
        target_id_ = closest_target;
        
        if (on_aggro_callback_) {
            on_aggro_callback_(target_id_);
        }
        
        return true;
    }
    
    return false;
}

bool NPC::checkDeaggro() {
    auto movement = getComponent<MovementComponent>();
    if (!movement) {
        return false;
    }
    
    if (target_id_ == 0) {
        return true;
    }
    
    Vector3 target_position(0.0f, 0.0f, 0.0f);
    float distance = (target_position - movement->getPosition()).length();
    
    if (distance > aggro_range_ * 2.0f) {
        if (on_deaggro_callback_) {
            on_deaggro_callback_();
        }
        return true;
    }
    
    return false;
}

void NPC::onDeath() {
    Character::onDeath();
    
    setAIState(AIState::DEAD);
    respawn_timer_ = respawn_delay_;
    
    std::cout << "NPC " << name_ << " has died!" << std::endl;
}

void NPC::respawn(float delay) {
    is_alive_ = true;
    setAIState(AIState::IDLE);
    
    auto movement = getComponent<MovementComponent>();
    if (movement) {
        movement->setPosition(spawn_position_);
        movement->stopMovement();
    }
    
    auto combat = getComponent<CombatComponent>();
    if (combat) {
        auto attrs = combat->getAttributes();
        attrs.health = attrs.max_health;
        attrs.mana = attrs.max_mana;
        combat->setAttributes(attrs);
        combat->setState(CharacterState::IDLE);
    }
    
    target_id_ = 0;
    
    std::cout << "NPC " << name_ << " has respawned!" << std::endl;
}

void NPC::addLoot(uint32_t item_id, int32_t count) {
    loot_table_.push_back(std::make_pair(item_id, count));
}

void NPC::clearLoot() {
    loot_table_.clear();
}

} 
} 

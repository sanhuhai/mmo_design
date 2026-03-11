#include "character/movement_component.h"
#include "character/character.h"
#include "character/combat_component.h"
#include <cmath>
#include <algorithm>

namespace game {
namespace character {

MovementComponent::MovementComponent(Character* owner)
    : Component(owner)
    , speed_(5.0f)
    , is_moving_(false)
    , is_jumping_(false)
    , jump_height_(0.0f)
    , jump_velocity_(10.0f)
    , current_path_index_(0) {
    
    position_ = Vector3(0.0f, 0.0f, 0.0f);
    velocity_ = Vector3(0.0f, 0.0f, 0.0f);
    target_position_ = Vector3(0.0f, 0.0f, 0.0f);
}

void MovementComponent::initialize() {
    Component::initialize();
    is_moving_ = false;
    is_jumping_ = false;
}

void MovementComponent::update(float delta_time) {
    Component::update(delta_time);
    
    if (!owner_->isAlive()) {
        stopMovement();
        return;
    }
    
    updateMovement(delta_time);
    updatePath(delta_time);
    
    if (is_jumping_) {
        jump_height_ += jump_velocity_ * delta_time;
        jump_velocity_ -= 20.0f * delta_time;
        
        if (jump_height_ <= 0.0f) {
            jump_height_ = 0.0f;
            is_jumping_ = false;
            jump_velocity_ = 10.0f;
        }
    }
}

void MovementComponent::updateMovement(float delta_time) {
    if (!is_moving_) {
        velocity_ = Vector3(0.0f, 0.0f, 0.0f);
        return;
    }
    
    Vector3 direction = target_position_ - position_;
    float distance = direction.length();
    
    if (distance < 0.1f) {
        position_ = target_position_;
        stopMovement();
        
        if (on_reach_target_callback_) {
            on_reach_target_callback_();
        }
        return;
    }
    
    direction.normalize();
    velocity_ = direction * speed_;
    
    Vector3 new_position = position_ + velocity_ * delta_time;
    
    if (!checkCollision(new_position)) {
        position_ = new_position;
        
        if (on_move_callback_) {
            on_move_callback_(position_);
        }
    } else {
        stopMovement();
    }
}

void MovementComponent::updatePath(float delta_time) {
    if (path_nodes_.empty()) {
        return;
    }
    
    if (current_path_index_ >= path_nodes_.size()) {
        clearPath();
        return;
    }
    
    Vector3 current_target = path_nodes_[current_path_index_];
    Vector3 direction = current_target - position_;
    float distance = direction.length();
    
    if (distance < 0.5f) {
        current_path_index_++;
        
        if (current_path_index_ >= path_nodes_.size()) {
            clearPath();
            return;
        }
    }
    
    target_position_ = path_nodes_[current_path_index_];
    is_moving_ = true;
}

void MovementComponent::moveTo(const Vector3& target) {
    target_position_ = target;
    is_moving_ = true;
    
    if (owner_) {
        auto combat = owner_->getComponent<CombatComponent>();
        if (combat) {
            combat->setState(CharacterState::MOVING);
        }
    }
}

void MovementComponent::stopMovement() {
    is_moving_ = false;
    velocity_ = Vector3(0.0f, 0.0f, 0.0f);
    
    if (owner_) {
        auto combat = owner_->getComponent<CombatComponent>();
        if (combat && combat->getCurrentState() == CharacterState::MOVING) {
            combat->setState(CharacterState::IDLE);
        }
    }
}

void MovementComponent::setPath(const std::vector<Vector3>& path) {
    path_nodes_ = path;
    current_path_index_ = 0;
    
    if (!path_nodes_.empty()) {
        target_position_ = path_nodes_[0];
        is_moving_ = true;
    }
}

void MovementComponent::clearPath() {
    path_nodes_.clear();
    current_path_index_ = 0;
    stopMovement();
}

void MovementComponent::jump() {
    if (!is_jumping_ && !is_moving_) {
        is_jumping_ = true;
        jump_velocity_ = 10.0f;
    }
}

bool MovementComponent::checkCollision(const Vector3& position) {
    return false;
}

Vector3 MovementComponent::getNextPathNode() {
    if (path_nodes_.empty() || current_path_index_ >= path_nodes_.size()) {
        return position_;
    }
    
    return path_nodes_[current_path_index_];
}

} 
} 

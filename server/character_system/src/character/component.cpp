#include "character/component.h"
#include "character/character.h"
#include <iostream>
#include <chrono>

namespace game {
namespace character {

uint64_t Component::next_component_id_ = 1;

Component::Component(Character* owner)
    : owner_(owner)
    , component_id_(next_component_id_++)
    , state_(ComponentState::UNINITIALIZED)
    , enabled_(true)
    , paused_(false)
    , priority_(ComponentPriority::NORMAL)
    , is_dirty_(false) {
}

Component::~Component() {
    event_listeners_.clear();
    dependencies_.clear();
    custom_data_.clear();
}

void Component::initialize() {
    if (state_ == ComponentState::UNINITIALIZED) {
        state_ = ComponentState::INITIALIZING;
        
        onEnable();
        
        state_ = ComponentState::INITIALIZED;
        state_ = ComponentState::ACTIVE;
    }
}

void Component::update(float delta_time) {
    if (!enabled_ || paused_ || state_ != ComponentState::ACTIVE) {
        return;
    }
}

void Component::destroy() {
    if (state_ == ComponentState::DESTROYED) {
        return;
    }
    
    state_ = ComponentState::DESTROYING;
    
    onDisable();
    
    event_listeners_.clear();
    dependencies_.clear();
    custom_data_.clear();
    
    state_ = ComponentState::DESTROYED;
}

void Component::onEnable() {
    enabled_ = true;
}

void Component::onDisable() {
    enabled_ = false;
}

void Component::onPause() {
    paused_ = true;
}

void Component::onResume() {
    paused_ = false;
}

void Component::setEnabled(bool enabled) {
    if (enabled_ == enabled) {
        return;
    }
    
    if (enabled) {
        onEnable();
    } else {
        onDisable();
    }
}

void Component::setPaused(bool paused) {
    if (paused_ == paused) {
        return;
    }
    
    if (paused) {
        onPause();
    } else {
        onResume();
    }
}

void Component::setPriority(ComponentPriority priority) {
    priority_ = priority;
}

void Component::addEventListener(const std::string& event_name, std::function<void(const ComponentEvent&)> callback) {
    event_listeners_[event_name] = callback;
}

void Component::removeEventListener(const std::string& event_name) {
    event_listeners_.erase(event_name);
}

void Component::triggerEvent(const std::string& event_name, void* data) {
    auto it = event_listeners_.find(event_name);
    if (it != event_listeners_.end()) {
        ComponentEvent event;
        event.event_name = event_name;
        event.data = data;
        
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        event.timestamp = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
        
        it->second(event);
    }
}

void Component::setDependency(const std::string& component_type) {
    dependencies_.push_back(component_type);
}

bool Component::hasDependency(const std::string& component_type) const {
    for (const auto& dep : dependencies_) {
        if (dep == component_type) {
            return true;
        }
    }
    return false;
}

std::string Component::serialize() const {
    return "";
}

bool Component::deserialize(const std::string& data) {
    return true;
}

Component* Component::clone(Character* new_owner) const {
    return nullptr;
}

void Component::debugPrint() const {
    std::cout << "Component ID: " << component_id_ << std::endl;
    std::cout << "Type: " << getTypeName() << std::endl;
    std::cout << "State: " << static_cast<int>(state_) << std::endl;
    std::cout << "Enabled: " << (enabled_ ? "true" : "false") << std::endl;
    std::cout << "Paused: " << (paused_ ? "true" : "false") << std::endl;
    std::cout << "Priority: " << static_cast<int>(priority_) << std::endl;
    std::cout << "Dirty: " << (is_dirty_ ? "true" : "false") << std::endl;
    std::cout << "Dependencies: " << dependencies_.size() << std::endl;
    std::cout << "Event Listeners: " << event_listeners_.size() << std::endl;
}

void Component::setCustomData(const std::string& key, void* data) {
    custom_data_[key] = data;
}

void* Component::getCustomData(const std::string& key) const {
    auto it = custom_data_.find(key);
    if (it != custom_data_.end()) {
        return it->second;
    }
    return nullptr;
}

void Component::removeCustomData(const std::string& key) {
    custom_data_.erase(key);
}

} 
} 

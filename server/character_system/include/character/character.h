#ifndef CHARACTER_H
#define CHARACTER_H

#include <string>
#include <cstdint>
#include <memory>
#include <map>
#include <typeinfo>
#include "character/character_types.h"

namespace game {
namespace character {

class Component;
class CombatComponent;
class MovementComponent;
class EquipmentComponent;
class StatusEffectComponent;
class NetworkSyncComponent;

class Character {
public:
    Character(uint64_t id, const std::string& name, CharacterType type);
    virtual ~Character();

    uint64_t getId() const { return id_; }
    const std::string& getName() const { return name_; }
    CharacterType getType() const { return type_; }
    bool isAlive() const { return is_alive_; }

    virtual void update(float delta_time);
    virtual void onDeath();

    template<typename T>
    T* getComponent() {
        size_t type_hash = typeid(T).hash_code();
        auto it = components_.find(type_hash);
        if (it != components_.end()) {
            return static_cast<T*>(it->second.get());
        }
        return nullptr;
    }

    template<typename T>
    void addComponent(std::unique_ptr<T> component) {
        size_t type_hash = typeid(T).hash_code();
        components_[type_hash] = std::move(component);
    }

    template<typename T>
    void removeComponent() {
        size_t type_hash = typeid(T).hash_code();
        components_.erase(type_hash);
    }

protected:
    uint64_t id_;
    std::string name_;
    CharacterType type_;
    bool is_alive_;

    std::map<size_t, std::unique_ptr<Component>> components_;
};

} 
} 

#endif 

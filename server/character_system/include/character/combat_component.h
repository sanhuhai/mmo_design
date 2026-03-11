#ifndef COMBAT_COMPONENT_H
#define COMBAT_COMPONENT_H

#include "character/component.h"
#include "character/character_types.h"
#include <vector>
#include <map>
#include <functional>
#include <memory>

namespace game {
namespace character {

struct Skill {
    uint32_t id;
    std::string name;
    float damage;
    float cooldown;
    float current_cooldown;
    float range;
    float cast_time;
};

struct DamageInfo {
    float physical_damage;
    float magic_damage;
    bool is_crit;
    float armor_penetration;
    float magic_penetration;
};

struct ThreatInfo {
    uint64_t target_id;
    float threat_value;
};

class CombatComponent : public Component {
public:
    CombatComponent(Character* owner);
    ~CombatComponent() override = default;

    void initialize() override;
    void update(float delta_time) override;

    CharacterState getCurrentState() const { return current_state_; }
    void setState(CharacterState state);

    bool canAttack() const;
    bool canCast() const;
    
    void attack(Character* target);
    void castSkill(uint32_t skill_id, Character* target);
    void takeDamage(const DamageInfo& damage);
    void heal(float amount);

    void addThreat(uint64_t target_id, float threat);
    uint64_t getHighestThreatTarget() const;
    void clearThreat();

    void addSkill(const Skill& skill);
    Skill* getSkill(uint32_t skill_id);
    const std::vector<Skill>& getSkills() const { return skills_; }

    BaseAttributes getAttributes() const { return attributes_; }
    void setAttributes(const BaseAttributes& attrs) { attributes_ = attrs; }

    DerivedAttributes getDerivedAttributes() const { return derived_attributes_; }
    void setDerivedAttributes(const DerivedAttributes& attrs) { derived_attributes_ = attrs; }

    void setOnDeathCallback(std::function<void()> callback) { on_death_callback_ = callback; }
    void setOnAttackCallback(std::function<void(Character*)> callback) { on_attack_callback_ = callback; }

private:
    void updateCooldowns(float delta_time);
    void updateThreat(float delta_time);
    DamageInfo calculateDamage(Character* target);
    void applyDamage(const DamageInfo& damage);
    void checkDeath();

private:
    CharacterState current_state_;
    BaseAttributes attributes_;
    DerivedAttributes derived_attributes_;
    
    std::vector<Skill> skills_;
    std::map<uint64_t, ThreatInfo> threat_map_;
    
    Character* current_target_;
    uint64_t current_skill_id_;
    float cast_timer_;
    
    std::function<void()> on_death_callback_;
    std::function<void(Character*)> on_attack_callback_;
};

} 
} 

#endif 

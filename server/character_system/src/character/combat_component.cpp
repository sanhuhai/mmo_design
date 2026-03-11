#include "character/combat_component.h"
#include "character/character.h"
#include <algorithm>
#include <cmath>

namespace game {
namespace character {

CombatComponent::CombatComponent(Character* owner)
    : Component(owner)
    , current_state_(CharacterState::IDLE)
    , current_target_(nullptr)
    , current_skill_id_(0)
    , cast_timer_(0.0f) {
    
    attributes_.health = 100.0f;
    attributes_.max_health = 100.0f;
    attributes_.mana = 50.0f;
    attributes_.max_mana = 50.0f;
    attributes_.stamina = 100.0f;
    attributes_.max_stamina = 100.0f;
    attributes_.attack = 10.0f;
    attributes_.defense = 5.0f;
    attributes_.magic_attack = 5.0f;
    attributes_.magic_defense = 3.0f;
    
    derived_attributes_.crit_rate = 0.05f;
    derived_attributes_.dodge_rate = 0.05f;
    derived_attributes_.hit_rate = 0.95f;
    derived_attributes_.move_speed = 5.0f;
    derived_attributes_.attack_speed = 1.0f;
    derived_attributes_.cast_speed = 1.0f;
}

void CombatComponent::initialize() {
    Component::initialize();
    setState(CharacterState::IDLE);
}

void CombatComponent::update(float delta_time) {
    Component::update(delta_time);
    
    updateCooldowns(delta_time);
    updateThreat(delta_time);
    
    if (cast_timer_ > 0.0f) {
        cast_timer_ -= delta_time;
        if (cast_timer_ <= 0.0f) {
            cast_timer_ = 0.0f;
            setState(CharacterState::IDLE);
        }
    }
}

void CombatComponent::setState(CharacterState state) {
    if (current_state_ == state) {
        return;
    }
    
    current_state_ = state;
    
    if (state == CharacterState::DEAD) {
        checkDeath();
    }
}

bool CombatComponent::canAttack() const {
    return current_state_ == CharacterState::IDLE && 
           attributes_.stamina >= 10.0f;
}

bool CombatComponent::canCast() const {
    return current_state_ == CharacterState::IDLE && 
           attributes_.mana >= 10.0f;
}

void CombatComponent::attack(Character* target) {
    if (!canAttack() || !target) {
        return;
    }
    
    current_target_ = target;
    setState(CharacterState::ATTACKING);
    
    DamageInfo damage = calculateDamage(target);
    target->getComponent<CombatComponent>()->takeDamage(damage);
    
    attributes_.stamina -= 10.0f;
    addThreat(target->getId(), damage.physical_damage + damage.magic_damage);
    
    if (on_attack_callback_) {
        on_attack_callback_(target);
    }
    
    setState(CharacterState::IDLE);
}

void CombatComponent::castSkill(uint32_t skill_id, Character* target) {
    if (!canCast() || !target) {
        return;
    }
    
    Skill* skill = getSkill(skill_id);
    if (!skill || skill->current_cooldown > 0.0f) {
        return;
    }
    
    current_target_ = target;
    current_skill_id_ = skill_id;
    cast_timer_ = skill->cast_time / derived_attributes_.cast_speed;
    setState(CharacterState::CASTING);
    
    DamageInfo damage;
    damage.physical_damage = skill->damage * attributes_.attack;
    damage.magic_damage = skill->damage * attributes_.magic_attack;
    damage.is_crit = (static_cast<float>(rand()) / RAND_MAX) < derived_attributes_.crit_rate;
    
    if (damage.is_crit) {
        damage.physical_damage *= 2.0f;
        damage.magic_damage *= 2.0f;
    }
    
    target->getComponent<CombatComponent>()->takeDamage(damage);
    
    skill->current_cooldown = skill->cooldown;
    attributes_.mana -= 10.0f;
    
    addThreat(target->getId(), damage.physical_damage + damage.magic_damage);
    
    if (on_attack_callback_) {
        on_attack_callback_(target);
    }
}

void CombatComponent::takeDamage(const DamageInfo& damage) {
    float physical_damage = std::max(0.0f, damage.physical_damage - attributes_.defense * (1.0f - damage.armor_penetration));
    float magic_damage = std::max(0.0f, damage.magic_damage - attributes_.magic_defense * (1.0f - damage.magic_penetration));
    
    float total_damage = physical_damage + magic_damage;
    attributes_.health -= total_damage;
    
    if (attributes_.health <= 0.0f) {
        attributes_.health = 0.0f;
        setState(CharacterState::DEAD);
    }
}

void CombatComponent::heal(float amount) {
    if (!owner_->isAlive()) {
        return;
    }
    
    attributes_.health = std::min(attributes_.max_health, attributes_.health + amount);
}

void CombatComponent::addThreat(uint64_t target_id, float threat) {
    auto it = threat_map_.find(target_id);
    if (it != threat_map_.end()) {
        it->second.threat_value += threat;
    } else {
        ThreatInfo info;
        info.target_id = target_id;
        info.threat_value = threat;
        threat_map_[target_id] = info;
    }
}

uint64_t CombatComponent::getHighestThreatTarget() const {
    if (threat_map_.empty()) {
        return 0;
    }
    
    auto it = std::max_element(threat_map_.begin(), threat_map_.end(),
        [](const auto& a, const auto& b) {
            return a.second.threat_value < b.second.threat_value;
        });
    
    return it->first;
}

void CombatComponent::clearThreat() {
    threat_map_.clear();
}

void CombatComponent::addSkill(const Skill& skill) {
    skills_.push_back(skill);
}

Skill* CombatComponent::getSkill(uint32_t skill_id) {
    for (auto& skill : skills_) {
        if (skill.id == skill_id) {
            return &skill;
        }
    }
    return nullptr;
}

void CombatComponent::updateCooldowns(float delta_time) {
    for (auto& skill : skills_) {
        if (skill.current_cooldown > 0.0f) {
            skill.current_cooldown -= delta_time;
            if (skill.current_cooldown < 0.0f) {
                skill.current_cooldown = 0.0f;
            }
        }
    }
}

void CombatComponent::updateThreat(float delta_time) {
    float decay_rate = 10.0f * delta_time;
    
    for (auto it = threat_map_.begin(); it != threat_map_.end(); ) {
        it->second.threat_value -= decay_rate;
        if (it->second.threat_value <= 0.0f) {
            it = threat_map_.erase(it);
        } else {
            ++it;
        }
    }
}

DamageInfo CombatComponent::calculateDamage(Character* target) {
    DamageInfo damage;
    
    float base_damage = attributes_.attack;
    damage.physical_damage = base_damage;
    damage.magic_damage = attributes_.magic_attack;
    damage.armor_penetration = 0.0f;
    damage.magic_penetration = 0.0f;
    
    damage.is_crit = (static_cast<float>(rand()) / RAND_MAX) < derived_attributes_.crit_rate;
    
    if (damage.is_crit) {
        damage.physical_damage *= 2.0f;
        damage.magic_damage *= 2.0f;
    }
    
    return damage;
}

void CombatComponent::applyDamage(const DamageInfo& damage) {
    float physical_damage = std::max(0.0f, damage.physical_damage - attributes_.defense);
    float magic_damage = std::max(0.0f, damage.magic_damage - attributes_.magic_defense);
    
    attributes_.health -= (physical_damage + magic_damage);
    
    if (attributes_.health <= 0.0f) {
        attributes_.health = 0.0f;
        checkDeath();
    }
}

void CombatComponent::checkDeath() {
    if (attributes_.health <= 0.0f && owner_->isAlive()) {
        owner_->onDeath();
        if (on_death_callback_) {
            on_death_callback_();
        }
    }
}

} 
} 

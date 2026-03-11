#ifndef NPC_H
#define NPC_H

#include "character/character.h"
#include "character/movement_component.h"
#include <string>
#include <memory>
#include <vector>

namespace game {
namespace character {

enum class NPCType {
    VENDOR = 0,
    QUEST_GIVER = 1,
    GUARD = 2,
    MONSTER = 3,
    PET = 4
};

enum class AIState {
    IDLE = 0,
    PATROL = 1,
    CHASE = 2,
    ATTACK = 3,
    FLEE = 4,
    DEAD = 5
};

class NPC : public Character {
public:
    NPC(uint64_t id, const std::string& name, NPCType type);
    ~NPC() override = default;

    NPCType getNPCType() const { return npc_type_; }
    void setNPCType(NPCType type) { npc_type_ = type; }

    AIState getAIState() const { return ai_state_; }
    void setAIState(AIState state) { ai_state_ = state; }

    uint64_t getTargetId() const { return target_id_; }
    void setTargetId(uint64_t target_id) { target_id_ = target_id; }

    float getAggroRange() const { return aggro_range_; }
    void setAggroRange(float range) { aggro_range_ = range; }

    float getAttackRange() const { return attack_range_; }
    void setAttackRange(float range) { attack_range_ = range; }

    float getPatrolRadius() const { return patrol_radius_; }
    void setPatrolRadius(float radius) { patrol_radius_ = radius; }

    const Vector3& getSpawnPosition() const { return spawn_position_; }
    void setSpawnPosition(const Vector3& pos) { spawn_position_ = pos; }

    void update(float delta_time) override;
    void onDeath() override;

    void respawn(float delay);

    void addLoot(uint32_t item_id, int32_t count);
    void clearLoot();

    void setOnAggroCallback(std::function<void(uint64_t)> callback) { on_aggro_callback_ = callback; }
    void setOnDeaggroCallback(std::function<void()> callback) { on_deaggro_callback_ = callback; }

private:
    void updateAI(float delta_time);
    void updatePatrol(float delta_time);
    void updateChase(float delta_time);
    void updateAttack(float delta_time);
    void updateFlee(float delta_time);

    bool checkAggro();
    bool checkDeaggro();

private:
    NPCType npc_type_;
    AIState ai_state_;
    
    uint64_t target_id_;
    float aggro_range_;
    float attack_range_;
    float patrol_radius_;
    
    Vector3 spawn_position_;
    Vector3 patrol_target_;
    float patrol_timer_;
    
    std::vector<std::pair<uint32_t, int32_t>> loot_table_;
    
    float respawn_delay_;
    float respawn_timer_;
    
    std::function<void(uint64_t)> on_aggro_callback_;
    std::function<void()> on_deaggro_callback_;
};

} 
} 

#endif 

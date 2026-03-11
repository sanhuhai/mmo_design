#ifndef CHARACTER_TYPES_H
#define CHARACTER_TYPES_H

#include <cstdint>
#include <cmath>

namespace game {
namespace character {

struct Vector3 {
    float x;
    float y;
    float z;
    
    Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
    
    Vector3 operator+(const Vector3& other) const {
        return Vector3(x + other.x, y + other.y, z + other.z);
    }
    
    Vector3 operator-(const Vector3& other) const {
        return Vector3(x - other.x, y - other.y, z - other.z);
    }
    
    Vector3 operator*(float scalar) const {
        return Vector3(x * scalar, y * scalar, z * scalar);
    }
    
    float length() const {
        return std::sqrt(x * x + y * y + z * z);
    }
    
    void normalize() {
        float len = length();
        if (len > 0.0f) {
            x /= len;
            y /= len;
            z /= len;
        }
    }
};

enum class CharacterType {
    PLAYER = 0,
    NPC = 1,
    MONSTER = 2,
    PET = 3
};

enum class CharacterState {
    IDLE = 0,
    MOVING = 1,
    ATTACKING = 2,
    CASTING = 3,
    HIT = 4,
    DEAD = 5
};

struct BaseAttributes {
    float health;
    float max_health;
    float mana;
    float max_mana;
    float stamina;
    float max_stamina;
    float attack;
    float defense;
    float magic_attack;
    float magic_defense;
};

struct GrowthAttributes {
    int32_t level;
    int64_t experience;
    int64_t max_experience;
    int32_t attribute_points;
    int32_t skill_points;
};

struct DerivedAttributes {
    float crit_rate;
    float dodge_rate;
    float hit_rate;
    float move_speed;
    float attack_speed;
    float cast_speed;
};

} 
} 

#endif 

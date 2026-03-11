#ifndef MOVEMENT_COMPONENT_H
#define MOVEMENT_COMPONENT_H

#include "character/component.h"
#include "character/character_types.h"
#include <vector>

namespace game {
namespace character {

struct PathNode {
    Vector3 position;
    float cost;
    PathNode* parent;
};

class MovementComponent : public Component {
public:
    MovementComponent(Character* owner);
    ~MovementComponent() override = default;

    void initialize() override;
    void update(float delta_time) override;

    const Vector3& getPosition() const { return position_; }
    void setPosition(const Vector3& pos) { position_ = pos; }

    const Vector3& getVelocity() const { return velocity_; }
    void setVelocity(const Vector3& vel) { velocity_ = vel; }

    const Vector3& getTargetPosition() const { return target_position_; }
    void setTargetPosition(const Vector3& pos) { target_position_ = pos; }

    float getSpeed() const { return speed_; }
    void setSpeed(float speed) { speed_ = speed; }

    bool isMoving() const { return is_moving_; }
    void setMoving(bool moving) { is_moving_ = moving; }

    void moveTo(const Vector3& target);
    void stopMovement();
    
    bool hasPath() const { return !path_nodes_.empty(); }
    void setPath(const std::vector<Vector3>& path);
    void clearPath();

    void jump();
    bool isJumping() const { return is_jumping_; }

    void setOnMoveCallback(std::function<void(const Vector3&)> callback) { on_move_callback_ = callback; }
    void setOnReachTargetCallback(std::function<void()> callback) { on_reach_target_callback_ = callback; }

private:
    void updateMovement(float delta_time);
    void updatePath(float delta_time);
    bool checkCollision(const Vector3& position);
    Vector3 getNextPathNode();

private:
    Vector3 position_;
    Vector3 velocity_;
    Vector3 target_position_;
    float speed_;
    
    bool is_moving_;
    bool is_jumping_;
    float jump_height_;
    float jump_velocity_;
    
    std::vector<Vector3> path_nodes_;
    size_t current_path_index_;
    
    std::function<void(const Vector3&)> on_move_callback_;
    std::function<void()> on_reach_target_callback_;
};

} 
} 

#endif 

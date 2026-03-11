#ifndef NETWORK_SYNC_COMPONENT_H
#define NETWORK_SYNC_COMPONENT_H

#include "character/component.h"
#include "character/character_types.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace game {
namespace character {

enum class SyncPriority {
    LOW = 0,
    MEDIUM = 1,
    HIGH = 2,
    CRITICAL = 3
};

struct SyncData {
    uint64_t character_id;
    Vector3 position;
    CharacterState state;
    float health;
    float max_health;
    float mana;
    float max_mana;
    uint32_t timestamp;
};

struct RPCRequest {
    uint32_t rpc_id;
    std::string method_name;
    std::vector<uint8_t> parameters;
    uint64_t target_id;
};

class NetworkSyncComponent : public Component {
public:
    NetworkSyncComponent(Character* owner);
    ~NetworkSyncComponent() override = default;

    void initialize() override;
    void update(float delta_time) override;

    void setSyncInterval(float interval) { sync_interval_ = interval; }
    float getSyncInterval() const { return sync_interval_; }

    void setSyncPriority(SyncPriority priority) { sync_priority_ = priority; }
    SyncPriority getSyncPriority() const { return sync_priority_; }

    void setDirty(bool dirty) { is_dirty_ = dirty; }
    bool isDirty() const { return is_dirty_; }

    SyncData getSyncData() const;
    void applySyncData(const SyncData& data);

    void markPositionDirty() { position_dirty_ = true; is_dirty_ = true; }
    void markStateDirty() { state_dirty_ = true; is_dirty_ = true; }
    void markAttributesDirty() { attributes_dirty_ = true; is_dirty_ = true; }

    void sendRPC(const std::string& method_name, const std::vector<uint8_t>& parameters, uint64_t target_id = 0);
    void handleRPC(const RPCRequest& request);

    void setOnSyncCallback(std::function<void(const SyncData&)> callback) { on_sync_callback_ = callback; }
    void setOnRPCCallback(std::function<void(const RPCRequest&)> callback) { on_rpc_callback_ = callback; }

    void enablePrediction(bool enable) { enable_prediction_ = enable; }
    bool isPredictionEnabled() const { return enable_prediction_; }

    void enableRollback(bool enable) { enable_rollback_ = enable; }
    bool isRollbackEnabled() const { return enable_rollback_; }

private:
    void updateSyncTimer(float delta_time);
    bool shouldSync() const;
    void performSync();

private:
    float sync_interval_;
    float sync_timer_;
    SyncPriority sync_priority_;
    bool is_dirty_;
    
    bool position_dirty_;
    bool state_dirty_;
    bool attributes_dirty_;
    
    bool enable_prediction_;
    bool enable_rollback_;
    
    SyncData last_sync_data_;
    std::vector<SyncData> prediction_history_;
    
    std::function<void(const SyncData&)> on_sync_callback_;
    std::function<void(const RPCRequest&)> on_rpc_callback_;
};

} 
} 

#endif 

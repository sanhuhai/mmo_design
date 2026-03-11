#pragma once

#include "redis/protobuf_serializer.h"

#ifdef ENABLE_REDIS
#include "redis/redis_client.h"
#endif

#ifdef ENABLE_DATABASE
#include "redis/database_client.h"
#endif

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <atomic>
#include <chrono>
#include <functional>
#include <csignal>
#include <thread>



// 前向声明
namespace game {
    class PlayerData;
    class PlayerBaseInfo;
    class PlayerInventoryInfo;
}

namespace redis {

class CacheManager {
public:
    static CacheManager* getInstance();
    
    bool initialize(const std::string& redis_host, int redis_port,
                   const std::string& db_host, int db_port,
                   const std::string& db_user, const std::string& db_password,
                   const std::string& db_name);
    
    void start();
    void stop();
    
    bool loadPlayerDataFromDB(int player_id);
    bool savePlayerDataToDB(int player_id);
    bool saveAllDirtyDataToDB();
    
    bool setPlayerData(int player_id, const game::PlayerData& data);
    bool getPlayerData(int player_id, game::PlayerData& data);
    
    bool setPlayerBaseInfo(int player_id, const game::PlayerBaseInfo& info);
    bool getPlayerBaseInfo(int player_id, game::PlayerBaseInfo& info);
    
    bool setPlayerInventory(int player_id, const game::PlayerInventoryInfo& info);
    bool getPlayerInventory(int player_id, game::PlayerInventoryInfo& info);
    
    void setDirty(int player_id);
    void clearDirty(int player_id);
    bool isDirty(int player_id);
    
    void setSyncInterval(int seconds);
    int getSyncInterval() const;
    
    void enableDBOnlyMode();
    void markAllDirty();
    
private:
    CacheManager();
    ~CacheManager();
    CacheManager(const CacheManager&) = delete;
    CacheManager& operator=(const CacheManager&) = delete;
    
    void startTimerTask();
    void startSignalHandler();
    static void signalHandler(int signum);
    
    bool syncPlayerDataToDB(int player_id);
    
    std::string getRedisKey(const std::string& prefix, int player_id);
    
#ifdef ENABLE_REDIS
    std::unique_ptr<RedisClient> redis_client_;
#endif

#ifdef ENABLE_DATABASE
    std::unique_ptr<DatabaseClient> db_client_;
#endif
    
    std::unique_ptr<ProtobufSerializer> serializer_;
    
    std::unordered_map<int, game::PlayerData> cache_data_;
    std::unordered_map<int, bool> dirty_flags_;
    std::mutex cache_mutex_;
    
    std::atomic<bool> running_;
    std::atomic<int> sync_interval_;  // 默认300秒
    std::atomic<bool> db_only_mode_;
    
    std::thread timer_thread_;
    std::thread signal_thread_;
};

} // namespace redis

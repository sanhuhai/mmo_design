#include "redis/cache_manager.h"
#include "player_data.pb.h"
#include <iostream>
#include <cstdlib>

#ifdef ENABLE_REDIS
#include "redis/redis_client.h"
#endif

#ifdef ENABLE_DATABASE
#include "redis/database_client.h"
#endif

namespace redis {

CacheManager* CacheManager::getInstance() {
    static CacheManager instance;
    return &instance;
}

CacheManager::CacheManager()
    : running_(false)
    , sync_interval_(300)
    , db_only_mode_(false) {
}

CacheManager::~CacheManager() {
    stop();
}

bool CacheManager::initialize(const std::string& redis_host, int redis_port,
                              const std::string& db_host, int db_port,
                              const std::string& db_user, const std::string& db_password,
                              const std::string& db_name) {
#ifdef ENABLE_REDIS
    redis_client_ = std::make_unique<RedisClient>();
    if (!redis_client_->connect(redis_host, redis_port)) {
        std::cerr << "Failed to connect to Redis: " << redis_client_->getLastError() << std::endl;
        // 即使Redis连接失败，也继续初始化数据库
    }
#endif
    
#ifdef ENABLE_DATABASE
    db_client_ = std::make_unique<DatabaseClient>();
    if (!db_client_->connect(db_host, db_port, db_user, db_password, db_name)) {
        std::cerr << "Failed to connect to database: " << db_client_->getLastError() << std::endl;
        return false;
    }
#endif
    
    serializer_ = std::make_unique<ProtobufSerializer>();
    
    return true;
}

void CacheManager::start() {
    running_ = true;
    startTimerTask();
    startSignalHandler();
}

void CacheManager::stop() {
    running_ = false;
    
    saveAllDirtyDataToDB();
    
    if (timer_thread_.joinable()) {
        timer_thread_.join();
    }
    
    if (signal_thread_.joinable()) {
        signal_thread_.join();
    }
    
#ifdef ENABLE_REDIS
    if (redis_client_) {
        redis_client_->disconnect();
    }
#endif

#ifdef ENABLE_DATABASE
    if (db_client_) {
        db_client_->disconnect();
    }
#endif
}

bool CacheManager::loadPlayerDataFromDB(int player_id) {
#ifdef ENABLE_DATABASE
    std::string serialized_data;
    if (!db_client_->selectPlayerData(player_id, serialized_data)) {
        std::cerr << "Failed to load player data from DB: " << db_client_->getLastError() << std::endl;
        return false;
    }
    
    game::PlayerData player_data;
    if (!serializer_->deserializePlayerData(serialized_data, player_data)) {
        std::cerr << "Failed to deserialize player data" << std::endl;
        return false;
    }
    
    std::lock_guard<std::mutex> lock(cache_mutex_);
    cache_data_[player_id] = player_data;
    dirty_flags_[player_id] = false;
    
#ifdef ENABLE_REDIS
    if (!db_only_mode_ && redis_client_) {
        std::string redis_key = getRedisKey("player:data", player_id);
        std::string redis_value = serializer_->serializePlayerData(player_data);
        if (!redis_client_->set(redis_key, redis_value)) {
            std::cerr << "Failed to set player data to Redis: " << redis_client_->getLastError() << std::endl;
        }
    }
#endif
    
    return true;
#else
    return false;
#endif
}

bool CacheManager::savePlayerDataToDB(int player_id) {
    return syncPlayerDataToDB(player_id);
}

bool CacheManager::saveAllDirtyDataToDB() {
#ifdef ENABLE_DATABASE
    std::lock_guard<std::mutex> lock(cache_mutex_);
    
    db_client_->beginTransaction();
    
    int success_count = 0;
    int fail_count = 0;
    
    for (auto& pair : dirty_flags_) {
        int pid = pair.first;
        bool dirty = pair.second;
        
        if (dirty) {
            if (syncPlayerDataToDB(pid)) {
                success_count++;
            } else {
                fail_count++;
            }
        }
    }
    
    db_client_->commitTransaction();
    
    std::cout << "Sync completed: " << success_count << " success, " << fail_count << " failed" << std::endl;
    
    return fail_count == 0;
#else
    return true;
#endif
}

bool CacheManager::setPlayerData(int player_id, const game::PlayerData& data) {
    std::lock_guard<std::mutex> lock(cache_mutex_);
    
    cache_data_[player_id] = data;
    dirty_flags_[player_id] = true;
    
#ifdef ENABLE_REDIS
    if (!db_only_mode_ && redis_client_) {
        std::string redis_key = getRedisKey("player:data", player_id);
        std::string redis_value = serializer_->serializePlayerData(data);
        if (!redis_client_->set(redis_key, redis_value)) {
            std::cerr << "Failed to set player data to Redis: " << redis_client_->getLastError() << std::endl;
        }
    }
#endif
    
    return true;
}

bool CacheManager::getPlayerData(int player_id, game::PlayerData& data) {
    std::lock_guard<std::mutex> lock(cache_mutex_);
    
    auto it = cache_data_.find(player_id);
    if (it != cache_data_.end()) {
        data = it->second;
        return true;
    }
    
#ifdef ENABLE_REDIS
    if (!db_only_mode_ && redis_client_) {
        std::string redis_key = getRedisKey("player:data", player_id);
        std::string redis_value = redis_client_->get(redis_key);
        
        if (!redis_value.empty()) {
            if (serializer_->deserializePlayerData(redis_value, data)) {
                cache_data_[player_id] = data;
                dirty_flags_[player_id] = false;
                return true;
            }
        }
    }
#endif
    
    // 从数据库加载
    return loadPlayerDataFromDB(player_id);
}

bool CacheManager::setPlayerBaseInfo(int player_id, const game::PlayerBaseInfo& info) {
    game::PlayerData data;
    if (!getPlayerData(player_id, data)) {
        // 如果不存在，创建新的玩家数据
        data.mutable_base_info()->CopyFrom(info);
        data.set_update_time(std::chrono::system_clock::now().time_since_epoch().count() / 1000);
        data.set_is_dirty(true);
    } else {
        data.mutable_base_info()->CopyFrom(info);
        data.set_update_time(std::chrono::system_clock::now().time_since_epoch().count() / 1000);
        data.set_is_dirty(true);
    }
    
    return setPlayerData(player_id, data);
}

bool CacheManager::getPlayerBaseInfo(int player_id, game::PlayerBaseInfo& info) {
    game::PlayerData data;
    if (!getPlayerData(player_id, data)) {
        return false;
    }
    
    info.CopyFrom(data.base_info());
    return true;
}

bool CacheManager::setPlayerInventory(int player_id, const game::PlayerInventoryInfo& info) {
    game::PlayerData data;
    if (!getPlayerData(player_id, data)) {
        // 如果不存在，创建新的玩家数据
        data.mutable_inventory_info()->CopyFrom(info);
        data.set_update_time(std::chrono::system_clock::now().time_since_epoch().count() / 1000);
        data.set_is_dirty(true);
    } else {
        data.mutable_inventory_info()->CopyFrom(info);
        data.set_update_time(std::chrono::system_clock::now().time_since_epoch().count() / 1000);
        data.set_is_dirty(true);
    }
    
    return setPlayerData(player_id, data);
}

bool CacheManager::getPlayerInventory(int player_id, game::PlayerInventoryInfo& info) {
    game::PlayerData data;
    if (!getPlayerData(player_id, data)) {
        return false;
    }
    
    info.CopyFrom(data.inventory_info());
    return true;
}

void CacheManager::setDirty(int player_id) {
    std::lock_guard<std::mutex> lock(cache_mutex_);
    dirty_flags_[player_id] = true;
}

void CacheManager::clearDirty(int player_id) {
    std::lock_guard<std::mutex> lock(cache_mutex_);
    dirty_flags_[player_id] = false;
}

bool CacheManager::isDirty(int player_id) {
    std::lock_guard<std::mutex> lock(cache_mutex_);
    auto it = dirty_flags_.find(player_id);
    if (it != dirty_flags_.end()) {
        return it->second;
    }
    return false;
}

void CacheManager::setSyncInterval(int seconds) {
    sync_interval_ = seconds;
}

int CacheManager::getSyncInterval() const {
    return sync_interval_;
}

void CacheManager::enableDBOnlyMode() {
    db_only_mode_ = true;
    std::cout << "Switched to DB-only mode" << std::endl;
}

void CacheManager::markAllDirty() {
    std::lock_guard<std::mutex> lock(cache_mutex_);
    for (auto& pair : dirty_flags_) {
        pair.second = true;
    }
    std::cout << "Marked all data as dirty" << std::endl;
}

void CacheManager::startTimerTask() {
    timer_thread_ = std::thread([this]() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(sync_interval_));
            
            if (!running_) break;
            
            saveAllDirtyDataToDB();
        }
    });
}

void CacheManager::startSignalHandler() {
    signal_thread_ = std::thread([this]() {
        signal(SIGINT, signalHandler);
        signal(SIGTERM, signalHandler);
        
        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });
}

void CacheManager::signalHandler(int signum) {
    if (signum == SIGINT || signum == SIGTERM) {
        std::cout << "Received signal " << signum << ", stopping cache manager..." << std::endl;
        getInstance()->stop();
        exit(0);
    }
}

bool CacheManager::syncPlayerDataToDB(int player_id) {
#ifdef ENABLE_DATABASE
    std::lock_guard<std::mutex> lock(cache_mutex_);
    
    auto it = cache_data_.find(player_id);
    if (it == cache_data_.end()) {
        return false;
    }
    
    const game::PlayerData& player_data = it->second;
    std::string serialized_data = serializer_->serializePlayerData(player_data);
    
    std::string check_sql = "SELECT player_id FROM player_data WHERE player_id = " + std::to_string(player_id);
    std::vector<std::vector<std::string>> results;
    
    bool success;
    if (db_client_->query(check_sql, results) && !results.empty()) {
        success = db_client_->updatePlayerData(player_id, serialized_data);
    } else {
        success = db_client_->insertPlayerData(player_id, serialized_data);
    }
    
    if (success) {
        dirty_flags_[player_id] = false;
    } else {
        std::cerr << "Failed to sync player data to DB: " << db_client_->getLastError() << std::endl;
    }
    
    return success;
#else
    return false;
#endif
}

std::string CacheManager::getRedisKey(const std::string& prefix, int player_id) {
    return prefix + ":" + std::to_string(player_id);
}

} // namespace redis

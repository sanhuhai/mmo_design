# C++ MMO大型游戏Redis缓存系统设计

## 1. 系统架构

### 整体架构
- **Redis缓存层**：高速内存缓存，存储热点数据
- **数据访问层**：统一的数据访问接口，屏蔽底层细节
- **数据库层**：持久化存储，使用MySQL
- **序列化层**：Proto3协议的数据序列化/反序列化
- **定时任务层**：定时同步缓存数据到数据库
- **事件处理层**：处理服务器异常和停服事件

### 模块关系
- **数据访问层** ↔ **Redis缓存层**：读写Redis缓存
- **数据访问层** ↔ **数据库层**：读写数据库
- **数据访问层** ↔ **序列化层**：序列化/反序列化数据
- **定时任务层** ↔ **数据访问层**：触发数据同步
- **事件处理层** ↔ **数据访问层**：处理异常时的数据同步

## 2. 核心流程

### 服务器启动流程
```
1. 初始化Redis连接
2. 初始化数据库连接
3. 从数据库加载所有数据
4. 使用Proto3反序列化数据
5. 将数据写入Redis缓存
6. 启动定时同步任务
7. 启动异常处理监听
```

### 数据写入流程
```
1. 接收数据写入请求
2. 序列化数据为Proto3格式
3. 写入Redis缓存
4. 更新脏标记
5. 返回写入结果
```

### 定时同步流程
```
1. 定时器触发（默认300秒）
2. 扫描所有脏数据
3. 对脏数据进行Proto3序列化
4. 写入数据库
5. 清除脏标记
6. 记录同步日志
```

### 异常/停服同步流程
```
1. 捕获异常/停服信号
2. 立即停止新数据写入
3. 扫描所有脏数据
4. 对数据进行Proto3序列化
5. 批量写入数据库
6. 清除Redis缓存
7. 关闭连接
8. 退出程序
```

## 3. Proto3数据结构定义

### 玩家基础信息
```protobuf
syntax = "proto3";

package game;

message PlayerBaseInfo {
    int32 player_id = 1;
    string username = 2;
    int32 level = 3;
    int64 experience = 4;
    int32 gold = 5;
    int32 silver = 6;
    int64 last_login_time = 7;
    int64 last_logout_time = 8;
    int32 status = 9;
}
```

### 玩家角色信息
```protobuf
message PlayerCharacterInfo {
    int32 player_id = 1;
    int32 character_id = 2;
    string character_name = 3;
    int32 profession = 4;
    int32 level = 5;
    int64 experience = 6;
    int32 hp = 7;
    int32 max_hp = 8;
    int32 mp = 9;
    int32 max_mp = 10;
    int32 attack = 11;
    int32 defense = 12;
    int32 speed = 13;
    Position position = 14;
    
    message Position {
        float x = 1;
        float y = 2;
        float z = 3;
        int32 map_id = 4;
    }
}
```

### 玩家背包信息
```protobuf
message PlayerInventoryInfo {
    int32 player_id = 1;
    int32 capacity = 2;
    repeated InventoryItem items = 3;
    
    message InventoryItem {
        int32 slot_id = 1;
        int32 item_id = 2;
        int32 count = 3;
        int32 durability = 4;
    }
}
```

### 玩家技能信息
```protobuf
message PlayerSkillInfo {
    int32 player_id = 1;
    repeated Skill skills = 2;
    
    message Skill {
        int32 skill_id = 1;
        int32 level = 2;
        int64 last_cast_time = 3;
    }
}
```

### 玩家好友信息
```protobuf
message PlayerFriendInfo {
    int32 player_id = 1;
    repeated Friend friends = 2;
    repeated FriendGroup groups = 3;
    
    message Friend {
        int32 friend_id = 1;
        string friend_name = 2;
        int32 group_id = 3;
        int64 friend_since = 4;
    }
    
    message FriendGroup {
        int32 group_id = 1;
        string group_name = 2;
    }
}
```

### 玩家任务信息
```protobuf
message PlayerQuestInfo {
    int32 player_id = 1;
    repeated Quest quests = 2;
    
    message Quest {
        int32 quest_id = 1;
        int32 status = 2;
        int32 progress = 3;
        int64 accept_time = 4;
        int64 complete_time = 5;
    }
}
```

### 完整玩家数据
```protobuf
message PlayerData {
    PlayerBaseInfo base_info = 1;
    PlayerCharacterInfo character_info = 2;
    PlayerInventoryInfo inventory_info = 3;
    PlayerSkillInfo skill_info = 4;
    PlayerFriendInfo friend_info = 5;
    PlayerQuestInfo quest_info = 6;
    int64 update_time = 7;
    bool is_dirty = 8;
}
```

### Redis缓存Key设计
```
Redis Key格式：
- 玩家基础数据：player:base:{player_id}
- 玩家角色数据：player:character:{player_id}
- 玩家背包数据：player:inventory:{player_id}
- 玩家技能数据：player:skill:{player_id}
- 玩家好友数据：player:friend:{player_id}
- 玩家任务数据：player:quest:{player_id}
- 玩家完整数据：player:data:{player_id}

Redis Value格式：
- 使用Proto3序列化的二进制数据
- 设置过期时间：-1（永不过期）
```

## 4. 核心实现

### 数据管理器
```cpp
#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <atomic>
#include <chrono>
#include <functional>
#include <csignal>

#include "redis_client.h"
#include "database_client.h"
#include "protobuf_serializer.h"
#include "player_data.pb.h"

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
    
    bool setPlayerData(int player_id, const PlayerData& data);
    bool getPlayerData(int player_id, PlayerData& data);
    
    bool setPlayerBaseInfo(int player_id, const PlayerBaseInfo& info);
    bool getPlayerBaseInfo(int player_id, PlayerBaseInfo& info);
    
    bool setPlayerInventory(int player_id, const PlayerInventoryInfo& info);
    bool getPlayerInventory(int player_id, PlayerInventoryInfo& info);
    
    void setDirty(int player_id);
    void clearDirty(int player_id);
    bool isDirty(int player_id);
    
    void setSyncInterval(int seconds);
    int getSyncInterval() const;
    
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
    
    std::unique_ptr<RedisClient> redis_client_;
    std::unique_ptr<DatabaseClient> db_client_;
    std::unique_ptr<ProtobufSerializer> serializer_;
    
    std::unordered_map<int, PlayerData> cache_data_;
    std::unordered_map<int, bool> dirty_flags_;
    std::mutex cache_mutex_;
    
    std::atomic<bool> running_;
    std::atomic<int> sync_interval_;  // 默认300秒
    
    std::thread timer_thread_;
    std::thread signal_thread_;
};
```

### Redis客户端封装
```cpp
#pragma once

#include <string>
#include <vector>
#include <memory>
#include <hiredis/hiredis.h>

class RedisClient {
public:
    RedisClient();
    ~RedisClient();
    
    bool connect(const std::string& host, int port);
    void disconnect();
    
    bool set(const std::string& key, const std::string& value);
    bool setex(const std::string& key, int seconds, const std::string& value);
    std::string get(const std::string& key);
    bool del(const std::string& key);
    bool exists(const std::string& key);
    
    bool hset(const std::string& key, const std::string& field, const std::string& value);
    std::string hget(const std::string& key, const std::string& field);
    bool hdel(const std::string& key, const std::string& field);
    std::vector<std::string> hgetall(const std::string& key);
    
    bool expire(const std::string& key, int seconds);
    int ttl(const std::string& key);
    
    std::string getLastError() const;
    
private:
    redisContext* context_;
    std::string last_error_;
};
```

### Proto3序列化器
```cpp
#pragma once

#include <string>
#include <memory>

class ProtobufSerializer {
public:
    ProtobufSerializer() = default;
    ~ProtobufSerializer() = default;
    
    template<typename T>
    std::string serialize(const T& message) {
        std::string data;
        if (!message.SerializeToString(&data)) {
            return "";
        }
        return data;
    }
    
    template<typename T>
    bool deserialize(const std::string& data, T& message) {
        return message.ParseFromString(data);
    }
    
    std::string serializePlayerData(const PlayerData& data);
    bool deserializePlayerData(const std::string& data, PlayerData& result);
    
    std::string serializePlayerBaseInfo(const PlayerBaseInfo& info);
    bool deserializePlayerBaseInfo(const std::string& data, PlayerBaseInfo& result);
    
    std::string serializePlayerInventory(const PlayerInventoryInfo& info);
    bool deserializePlayerInventory(const std::string& data, PlayerInventoryInfo& result);
};
```

### 数据库客户端封装
```cpp
#pragma once

#include <string>
#include <vector>
#include <memory>
#include <mysql/mysql.h>

class DatabaseClient {
public:
    DatabaseClient();
    ~DatabaseClient();
    
    bool connect(const std::string& host, int port,
                 const std::string& user, const std::string& password,
                 const std::string& db_name);
    void disconnect();
    
    bool execute(const std::string& sql);
    bool query(const std::string& sql, std::vector<std::vector<std::string>>& results);
    
    bool insertPlayerData(int player_id, const std::string& serialized_data);
    bool updatePlayerData(int player_id, const std::string& serialized_data);
    bool selectPlayerData(int player_id, std::string& serialized_data);
    bool deletePlayerData(int player_id);
    
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();
    
    std::string getLastError() const;
    
private:
    MYSQL* connection_;
    std::string last_error_;
    
    bool isConnected();
};
```

### 数据管理器实现
```cpp
#include "cache_manager.h"

CacheManager* CacheManager::getInstance() {
    static CacheManager instance;
    return &instance;
}

CacheManager::CacheManager()
    : running_(false)
    , sync_interval_(300) {
}

CacheManager::~CacheManager() {
    stop();
}

bool CacheManager::initialize(const std::string& redis_host, int redis_port,
                              const std::string& db_host, int db_port,
                              const std::string& db_user, const std::string& db_password,
                              const std::string& db_name) {
    redis_client_ = std::make_unique<RedisClient>();
    if (!redis_client_->connect(redis_host, redis_port)) {
        return false;
    }
    
    db_client_ = std::make_unique<DatabaseClient>();
    if (!db_client_->connect(db_host, db_port, db_user, db_password, db_name)) {
        return false;
    }
    
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
    
    redis_client_->disconnect();
    db_client_->disconnect();
}

bool CacheManager::loadPlayerDataFromDB(int player_id) {
    std::string serialized_data;
    if (!db_client_->selectPlayerData(player_id, serialized_data)) {
        return false;
    }
    
    PlayerData player_data;
    if (!serializer_->deserializePlayerData(serialized_data, player_data)) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(cache_mutex_);
    cache_data_[player_id] = player_data;
    dirty_flags_[player_id] = false;
    
    std::string redis_key = getRedisKey("player:data", player_id);
    std::string redis_value = serializer_->serializePlayerData(player_data);
    redis_client_->set(redis_key, redis_value);
    
    return true;
}

bool CacheManager::savePlayerDataToDB(int player_id) {
    return syncPlayerDataToDB(player_id);
}

bool CacheManager::saveAllDirtyDataToDB() {
    std::lock_guard<std::mutex> lock(cache_mutex_);
    
    db_client_->beginTransaction();
    
    for (auto& pair : dirty_flags_) {
        int pid = pair.first;
        bool dirty = pair.second;
        
        if (dirty) {
            syncPlayerDataToDB(pid);
        }
    }
    
    db_client_->commitTransaction();
    
    return true;
}

bool CacheManager::setPlayerData(int player_id, const PlayerData& data) {
    std::lock_guard<std::mutex> lock(cache_mutex_);
    
    cache_data_[player_id] = data;
    dirty_flags_[player_id] = true;
    
    std::string redis_key = getRedisKey("player:data", player_id);
    std::string redis_value = serializer_->serializePlayerData(data);
    redis_client_->set(redis_key, redis_value);
    
    return true;
}

bool CacheManager::getPlayerData(int player_id, PlayerData& data) {
    std::lock_guard<std::mutex> lock(cache_mutex_);
    
    auto it = cache_data_.find(player_id);
    if (it != cache_data_.end()) {
        data = it->second;
        return true;
    }
    
    std::string redis_key = getRedisKey("player:data", player_id);
    std::string redis_value = redis_client_->get(redis_key);
    
    if (redis_value.empty()) {
        return false;
    }
    
    if (!serializer_->deserializePlayerData(redis_value, data)) {
        return false;
    }
    
    cache_data_[player_id] = data;
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
        getInstance()->stop();
        exit(0);
    }
}

bool CacheManager::syncPlayerDataToDB(int player_id) {
    std::lock_guard<std::mutex> lock(cache_mutex_);
    
    auto it = cache_data_.find(player_id);
    if (it == cache_data_.end()) {
        return false;
    }
    
    const PlayerData& player_data = it->second;
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
    }
    
    return success;
}

std::string CacheManager::getRedisKey(const std::string& prefix, int player_id) {
    return prefix + ":" + std::to_string(player_id);
}
```

## 5. 数据库表结构

### 玩家数据表
```sql
CREATE TABLE `player_data` (
    `player_id` INT NOT NULL PRIMARY KEY,
    `data` BLOB NOT NULL COMMENT 'Proto3序列化的玩家数据',
    `update_time` BIGINT NOT NULL COMMENT '更新时间戳',
    `create_time` BIGINT NOT NULL COMMENT '创建时间戳',
    INDEX `idx_update_time` (`update_time`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='玩家数据表';

CREATE TABLE `player_dirty_log` (
    `id` BIGINT NOT NULL AUTO_INCREMENT PRIMARY KEY,
    `player_id` INT NOT NULL COMMENT '玩家ID',
    `dirty_time` BIGINT NOT NULL COMMENT '脏数据时间戳',
    `sync_time` BIGINT DEFAULT NULL COMMENT '同步时间戳',
    INDEX `idx_player_dirty_time` (`player_id`, `dirty_time`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='玩家脏数据日志表';

CREATE TABLE `sync_status` (
    `id` BIGINT NOT NULL AUTO_INCREMENT PRIMARY KEY,
    `last_sync_time` BIGINT NOT NULL COMMENT '上次同步时间戳',
    `sync_count` INT NOT NULL DEFAULT 0 COMMENT '同步次数',
    `last_error` TEXT COMMENT '上次错误信息',
    `update_time` BIGINT NOT NULL COMMENT '更新时间戳'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='同步状态表';
```

## 6. 使用示例

### 初始化和使用
```cpp
#include <iostream>
#include "cache_manager.h"
#include "player_data.pb.h"

int main() {
    CacheManager* cache = CacheManager::getInstance();
    
    if (!cache->initialize("127.0.0.1", 6379,
                          "localhost", 3306,
                          "root", "password",
                          "game_db")) {
        std::cerr << "Failed to initialize cache manager" << std::endl;
        return -1;
    }
    
    cache->start();
    
    int player_id = 10001;
    
    if (cache->loadPlayerDataFromDB(player_id)) {
        PlayerData data;
        if (cache->getPlayerData(player_id, data)) {
            std::cout << "Loaded player: " << data.base_info().username() 
                      << ", Level: " << data.base_info().level() << std::endl;
            
            data.mutable_base_info()->set_gold(data.base_info().gold() + 100);
            data.set_update_time(time(nullptr));
            data.set_is_dirty(true);
            
            cache->setPlayerData(player_id, data);
        }
    }
    
    cache->setSyncInterval(300);
    
    std::this_thread::sleep_for(std::chrono::seconds(10));
    
    cache->stop();
    
    return 0;
}
```

## 7. 异常处理

### 异常类型
- **Redis连接异常**：自动重连，切换到数据库直接访问
- **数据库连接异常**：缓存数据，延迟写入
- **序列化异常**：记录错误，返回失败
- **内存不足**：清理过期缓存，限制新数据写入

### 处理策略
```cpp
class ExceptionHandler {
public:
    static void handleRedisException(const std::exception& e) {
        log.error("Redis exception: %s", e.what());
        
        CacheManager::getInstance()->enableDBOnlyMode();
    }
    
    static void handleDatabaseException(const std::exception& e) {
        log.error("Database exception: %s", e.what());
        
        CacheManager::getInstance()->markAllDirty();
    }
    
    static void handleSerializationException(const std::exception& e) {
        log.error("Serialization exception: %s", e.what());
    }
};
```

## 8. 性能优化

### 缓存优化
- **LRU缓存淘汰**：限制缓存数量，使用LRU淘汰
- **分片存储**：使用Redis集群，分片存储数据
- **Pipeline批量操作**：使用Pipeline批量操作，减少网络开销

### 同步优化
- **增量同步**：只同步变化的数据，而不是全部数据
- **批量同步**：合并多个玩家的数据，一次性写入
- **异步同步**：后台线程同步，不阻塞主线程

### 数据库优化
- **批量写入**：合并多个更新为一条SQL
- **索引优化**：为常用查询创建索引
- **读写分离**：使用读写分离的数据库架构

## 9. 监控与日志

### 监控指标
- **缓存命中率**：Redis缓存命中率
- **同步延迟**：数据同步到数据库的延迟
- **脏数据数量**：当前脏数据的数量
- **同步失败次数**：数据同步失败的次数

### 日志记录
```cpp
class SyncLogger {
public:
    static void logSyncStart(int player_count) {
        log.info("Sync started, player_count: %d", player_count);
    }
    
    static void logSyncComplete(int success_count, int fail_count) {
        log.info("Sync completed, success: %d, failed: %d", 
                 success_count, fail_count);
    }
    
    static void logSyncError(int player_id, const std::string& error) {
        log.error("Sync error, player_id: %d, error: %s", 
                  player_id, error.c_str());
    }
};
```

## 10. 配置参数

### 配置文件
```yaml
redis:
  host: "127.0.0.1"
  port: 6379
  password: ""
  db: 0
  pool_size: 10

database:
  host: "localhost"
  port: 3306
  user: "root"
  password: "password"
  database: "game_db"
  pool_size: 10

sync:
  interval: 300  # 同步间隔（秒）
  batch_size: 100  # 批量同步大小
  enable_auto_sync: true  # 是否启用自动同步

cache:
  max_player_count: 10000  # 最大缓存玩家数量
  enable_lru: true  # 是否启用LRU淘汰
```

本设计方案实现了一个完整的C++ MMO游戏Redis缓存系统，具备以下特点：

1. **三级数据存储**：Redis缓存 + 数据库持久化 + Proto3序列化
2. **自动同步机制**：定时同步（默认300秒）+ 异常/停服同步
3. **高效序列化**：使用Proto3协议进行二进制序列化/反序列化
4. **完善的异常处理**：连接异常、数据异常、内存异常等处理
5. **性能优化**：LRU淘汰、批量操作、Pipeline优化
6. **监控与日志**：完整的监控指标和日志记录
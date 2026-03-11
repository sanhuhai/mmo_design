#include "redis/cache_manager.h"
#include "player_data.pb.h"

#include <iostream>
#include <thread>
#include <chrono>


int main() {
    std::cout << "=== Redis Cache System Example ===" << std::endl;
    
    redis::CacheManager* cache = redis::CacheManager::getInstance();
    
    // 初始化缓存管理器
    if (!cache->initialize("127.0.0.1", 6379,  // Redis配置
                          "localhost", 3306,  // MySQL配置
                          "root", "password", "game_db")) {
        std::cerr << "Failed to initialize cache manager" << std::endl;
        return -1;
    }
    
    cache->start();
    std::cout << "Cache manager started" << std::endl;
    
    int player_id = 10001;
    
    // 创建测试玩家数据
    game::PlayerData player_data;
    auto* base_info = player_data.mutable_base_info();
    base_info->set_player_id(player_id);
    base_info->set_username("TestPlayer");
    base_info->set_level(1);
    base_info->set_experience(0);
    base_info->set_gold(1000);
    base_info->set_silver(10000);
    base_info->set_last_login_time(std::chrono::system_clock::now().time_since_epoch().count() / 1000);
    base_info->set_status(1);
    
    auto* character_info = player_data.mutable_character_info();
    character_info->set_player_id(player_id);
    character_info->set_character_id(1);
    character_info->set_character_name("Warrior");
    character_info->set_profession(1);
    character_info->set_level(1);
    character_info->set_experience(0);
    character_info->set_hp(100);
    character_info->set_max_hp(100);
    character_info->set_mp(50);
    character_info->set_max_mp(50);
    character_info->set_attack(10);
    character_info->set_defense(5);
    character_info->set_speed(3);
    
    auto* position = character_info->mutable_position();
    position->set_x(100.0f);
    position->set_y(200.0f);
    position->set_z(0.0f);
    position->set_map_id(1);
    
    player_data.set_update_time(std::chrono::system_clock::now().time_since_epoch().count() / 1000);
    player_data.set_is_dirty(true);
    
    // 保存玩家数据
    if (cache->setPlayerData(player_id, player_data)) {
        std::cout << "Player data saved successfully" << std::endl;
    } else {
        std::cerr << "Failed to save player data" << std::endl;
    }
    
    // 读取玩家数据
    game::PlayerData loaded_data;
    if (cache->getPlayerData(player_id, loaded_data)) {
        std::cout << "Loaded player: " << loaded_data.base_info().username() 
                  << ", Level: " << loaded_data.base_info().level() 
                  << ", Gold: " << loaded_data.base_info().gold() << std::endl;
    } else {
        std::cerr << "Failed to load player data" << std::endl;
    }
    
    // 更新玩家数据
    loaded_data.mutable_base_info()->set_gold(loaded_data.base_info().gold() + 500);
    loaded_data.set_update_time(std::chrono::system_clock::now().time_since_epoch().count() / 1000);
    loaded_data.set_is_dirty(true);
    
    if (cache->setPlayerData(player_id, loaded_data)) {
        std::cout << "Player data updated successfully" << std::endl;
    } else {
        std::cerr << "Failed to update player data" << std::endl;
    }
    
    // 再次读取验证更新
    game::PlayerData updated_data;
    if (cache->getPlayerData(player_id, updated_data)) {
        std::cout << "Updated player gold: " << updated_data.base_info().gold() << std::endl;
    }
    
    // 测试同步间隔
    cache->setSyncInterval(10); // 10秒同步一次
    std::cout << "Sync interval set to 10 seconds" << std::endl;
    
    // 等待一段时间，观察同步
    std::cout << "Waiting for 15 seconds to test auto-sync..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(15));
    
    // 手动同步测试
    std::cout << "Manual sync test..." << std::endl;
    if (cache->saveAllDirtyDataToDB()) {
        std::cout << "Manual sync completed successfully" << std::endl;
    } else {
        std::cerr << "Manual sync failed" << std::endl;
    }
    
    // 测试脏数据标记
    std::cout << "Dirty flag test: " << (cache->isDirty(player_id) ? "Dirty" : "Clean") << std::endl;
    
    // 停止缓存管理器
    std::cout << "Stopping cache manager..." << std::endl;
    cache->stop();
    
    std::cout << "Example completed" << std::endl;
    return 0;
}

#include "game/user_manager.h"
#include <random>
#include <chrono>
#include <sstream>
#include <iomanip>

namespace game {

UserManager* UserManager::getInstance() {
    static UserManager instance;
    return &instance;
}

UserManager::UserManager() : next_user_id_(1) {
}

UserManager::~UserManager() {
}

bool UserManager::registerUser(const std::string& username, const std::string& password, 
                              const std::string& email, const std::string& phone, int& user_id) {
    std::lock_guard<std::mutex> lock(users_mutex_);
    
    // 检查用户名是否已存在
    for (const auto& pair : users_) {
        if (pair.second.username == username) {
            return false;
        }
    }
    
    // 创建新用户
    User user;
    user.user_id = next_user_id_++;
    user.username = username;
    user.password = password; // 实际项目中应该加密存储
    user.email = email;
    user.phone = phone;
    
    users_[user.user_id] = user;
    user_id = user.user_id;
    return true;
}

bool UserManager::loginUser(const std::string& username, const std::string& password, 
                          User& user, std::string& token) {
    std::lock_guard<std::mutex> lock(users_mutex_);
    
    // 查找用户
    for (const auto& pair : users_) {
        if (pair.second.username == username && pair.second.password == password) {
            user = pair.second;
            token = generateToken(user.user_id);
            users_[user.user_id].token = token;
            return true;
        }
    }
    
    return false;
}

bool UserManager::validateToken(int user_id, const std::string& token) {
    std::lock_guard<std::mutex> lock(users_mutex_);
    
    auto it = users_.find(user_id);
    if (it != users_.end()) {
        return it->second.token == token;
    }
    
    return false;
}

User* UserManager::getUser(int user_id) {
    std::lock_guard<std::mutex> lock(users_mutex_);
    
    auto it = users_.find(user_id);
    if (it != users_.end()) {
        return &it->second;
    }
    
    return nullptr;
}

std::string UserManager::generateToken(int user_id) {
    // 生成简单的token，实际项目中应该使用更安全的方法
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(1000, 9999);
    int random = dist(gen);
    
    std::stringstream ss;
    ss << user_id << "-" << timestamp << "-" << random;
    return ss.str();
}

} // namespace game

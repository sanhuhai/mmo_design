#pragma once

#include <string>
#include <unordered_map>
#include <mutex>

namespace game {

struct User {
    int user_id;
    std::string username;
    std::string password;
    std::string email;
    std::string phone;
    std::string token;
};

class UserManager {
public:
    static UserManager* getInstance();
    
    bool registerUser(const std::string& username, const std::string& password, 
                     const std::string& email, const std::string& phone, int& user_id);
    
    bool loginUser(const std::string& username, const std::string& password, 
                   User& user, std::string& token);
    
    bool validateToken(int user_id, const std::string& token);
    
    User* getUser(int user_id);
    
private:
    UserManager();
    ~UserManager();
    
    std::unordered_map<int, User> users_;
    std::mutex users_mutex_;
    int next_user_id_;
    
    std::string generateToken(int user_id);
};

} // namespace game

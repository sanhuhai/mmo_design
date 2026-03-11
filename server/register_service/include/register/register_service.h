#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <atomic>

#include "register/validation_service.h"
#include "register/verification_code_service.h"

#ifdef PROTOBUF_FOUND
#include "register.pb.h"
#endif

#ifdef ENABLE_REDIS
#include "redis/redis_client.h"
#endif

#ifdef ENABLE_DATABASE
#include "redis/database_client.h"
#endif

namespace register_service {

enum RegisterErrorCode {
    SUCCESS = 0,
    
    // 客户端错误
    ERROR_INVALID_USERNAME = 1001,    // 用户名格式错误
    ERROR_INVALID_PASSWORD = 1002,    // 密码格式错误
    ERROR_INVALID_PHONE = 1003,       // 手机号格式错误
    ERROR_INVALID_EMAIL = 1004,      // 邮箱格式错误
    ERROR_INVALID_CODE = 1005,        // 验证码错误
    
    // 业务错误
    ERROR_USERNAME_EXISTS = 2001,    // 用户名已存在
    ERROR_PHONE_EXISTS = 2002,       // 手机号已注册
    ERROR_EMAIL_EXISTS = 2003,       // 邮箱已注册
    ERROR_CODE_EXPIRED = 2004,       // 验证码已过期
    ERROR_CODE_USED = 2005,          // 验证码已使用
    ERROR_TOO_MANY_REQUESTS = 2006,  // 请求过于频繁
    ERROR_REGISTER_LIMIT = 2007,     // 注册次数超限
    
    // 服务端错误
    ERROR_SEND_FAILED = 3001,        // 发送失败
    ERROR_CREATE_USER = 3002,         // 创建用户失败
    ERROR_DATABASE = 3003,           // 数据库错误
    ERROR_INTERNAL = 3004,          // 内部错误
};

class RegisterService {
public:
    static RegisterService* getInstance();
    
    bool initialize(const std::string& redis_host, int redis_port,
                   const std::string& db_host, int db_port,
                   const std::string& db_user, const std::string& db_password,
                   const std::string& db_name);
    
    #ifdef PROTOBUF_FOUND
    int sendVerificationCode(const SendCodeRequest& request, SendCodeResponse& response);
    int registerAccount(const RegisterRequest& request, RegisterResponse& response);
    int activateAccount(const ActivateRequest& request, ActivateResponse& response);
    #endif
    int sendVerificationCode(const std::string& request, std::string& response);
    int registerAccount(const std::string& request, std::string& response);
    int activateAccount(const std::string& request, std::string& response);
    
private:
    RegisterService();
    ~RegisterService();
    
    bool validateUsername(const std::string& username);
    bool validatePassword(const std::string& password);
    bool validatePhone(const std::string& phone);
    bool validateEmail(const std::string& email);
    
    bool checkUsernameExists(const std::string& username);
    bool checkPhoneExists(const std::string& phone);
    bool checkEmailExists(const std::string& email);
    
    bool checkSendFrequency(const std::string& contact, int type);
    bool checkRegisterLimit(const std::string& ip, const std::string& device_id);
    
    std::string generateVerificationCode();
    std::string generateToken(int user_id);
    std::string encryptPassword(const std::string& password);
    
    #ifdef PROTOBUF_FOUND
    int createUser(const RegisterRequest& request, int& user_id);
    bool giveRegisterReward(int user_id, RegisterReward& reward);
    #endif
    int createUser(const std::string& request, int& user_id);
    bool giveRegisterReward(int user_id, std::string& reward);
    
    std::string getLastError() const;
    
private:
    #ifdef ENABLE_REDIS
    std::unique_ptr<redis::RedisClient> redis_client_;
    #endif
    #ifdef ENABLE_DATABASE
    // 暂时注释掉，因为 redis::DatabaseClient 类未定义
    // std::unique_ptr<redis::DatabaseClient> db_client_;
    #endif
    std::unique_ptr<ValidationService> validation_service_;
    std::unique_ptr<VerificationCodeService> verification_code_service_;
    
    std::mutex register_mutex_;
    std::atomic<int> request_count_;
    
    std::unordered_map<std::string, std::string> config_;
    std::string last_error_;
};

} // namespace register_service
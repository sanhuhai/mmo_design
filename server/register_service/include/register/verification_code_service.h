#pragma once

#include <string>
#include <unordered_map>

#ifdef ENABLE_REDIS
#include "redis/redis_client.h"
#endif

#ifdef ENABLE_REDIS
namespace redis {
    class RedisClient;
}
#endif

namespace register_service {

struct VerificationCode {
    std::string code;           // 验证码
    std::string phone;           // 手机号/邮箱
    int type;                   // 验证码类型
    long create_time;           // 创建时间
    long expire_time;           // 过期时间
    int send_count;             // 发送次数
};

class VerificationCodeService {
public:
    VerificationCodeService() = default;
    ~VerificationCodeService() = default;
    
    #ifdef ENABLE_REDIS
    bool initialize(std::unique_ptr<redis::RedisClient>& redis_client);
    #else
    bool initialize();
    #endif
    
    bool sendCode(const std::string& contact, int type, int& error_code, std::string& error_msg);
    bool verifyCode(const std::string& contact, const std::string& code, int type);
    bool deleteCode(const std::string& contact, int type);
    
private:
    std::string generateCode();
    bool saveToCache(const VerificationCode& code);
    bool getFromCache(const std::string& contact, int type, VerificationCode& code);
    std::string getCacheKey(const std::string& contact, int type);
    std::string getFrequencyKey(const std::string& contact);
    
    static const int CODE_LENGTH = 6;
    static const int CODE_EXPIRE_TIME = 300;  // 5分钟
    static const int MAX_SEND_COUNT_PER_DAY = 10;
    static const int MIN_SEND_INTERVAL = 60;  // 60秒
    
    #ifdef ENABLE_REDIS
    std::unique_ptr<redis::RedisClient> redis_client_;
    #endif
};

} // namespace register_service
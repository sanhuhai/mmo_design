#include "register/verification_code_service.h"
#include "register/register_service.h"
#include <cstdlib>
#include <ctime>

#ifdef ENABLE_REDIS
#include "redis/redis_client.h"
#endif

namespace register_service {

#ifdef ENABLE_REDIS
bool VerificationCodeService::initialize(std::unique_ptr<redis::RedisClient>& redis_client) {
    redis_client_ = std::move(redis_client);
    srand(time(nullptr));
    return true;
}
#else
bool VerificationCodeService::initialize() {
    srand(time(nullptr));
    return true;
}
#endif

bool VerificationCodeService::sendCode(const std::string& contact, int type, int& error_code, std::string& error_msg) {
    // 检查发送频率
    std::string frequency_key = getFrequencyKey(contact);
    std::string count_str = redis_client_->get(frequency_key);
    int count = 0;
    if (!count_str.empty()) {
        count = std::stoi(count_str);
    }
    
    if (count >= MAX_SEND_COUNT_PER_DAY) {
        error_code = ERROR_TOO_MANY_REQUESTS;
        error_msg = "Too many requests, please try again tomorrow";
        return false;
    }
    
    // 生成验证码
    std::string code = generateCode();
    
    // 保存验证码到Redis
    VerificationCode verification_code;
    verification_code.code = code;
    verification_code.phone = contact;
    verification_code.type = type;
    verification_code.create_time = time(nullptr);
    verification_code.expire_time = verification_code.create_time + CODE_EXPIRE_TIME;
    verification_code.send_count = count + 1;
    
    if (!saveToCache(verification_code)) {
        error_code = ERROR_INTERNAL;
        error_msg = "Failed to save verification code";
        return false;
    }
    
    // 更新发送频率
    redis_client_->setex(frequency_key, 86400, std::to_string(verification_code.send_count));
    
    // 这里应该调用短信或邮件服务发送验证码
    // 简化实现，实际项目中需要集成短信/邮件服务
    
    return true;
}

bool VerificationCodeService::verifyCode(const std::string& contact, const std::string& code, int type) {
    VerificationCode verification_code;
    if (!getFromCache(contact, type, verification_code)) {
        return false;
    }
    
    if (verification_code.code != code) {
        return false;
    }
    
    if (time(nullptr) > verification_code.expire_time) {
        return false;
    }
    
    return true;
}

bool VerificationCodeService::deleteCode(const std::string& contact, int type) {
    std::string key = getCacheKey(contact, type);
    return redis_client_->del(key);
}

std::string VerificationCodeService::generateCode() {
    static const char* chars = "0123456789";
    std::string code;
    for (int i = 0; i < CODE_LENGTH; i++) {
        code += chars[rand() % 10];
    }
    return code;
}

bool VerificationCodeService::saveToCache(const VerificationCode& code) {
    std::string key = getCacheKey(code.phone, code.type);
    std::string value = code.code;
    return redis_client_->setex(key, CODE_EXPIRE_TIME, value);
}

bool VerificationCodeService::getFromCache(const std::string& contact, int type, VerificationCode& code) {
    std::string key = getCacheKey(contact, type);
    std::string value = redis_client_->get(key);
    if (value.empty()) {
        return false;
    }
    
    code.code = value;
    code.phone = contact;
    code.type = type;
    code.create_time = time(nullptr) - CODE_EXPIRE_TIME / 2; // 简化实现
    code.expire_time = time(nullptr) + CODE_EXPIRE_TIME / 2; // 简化实现
    code.send_count = 1; // 简化实现
    
    return true;
}

std::string VerificationCodeService::getCacheKey(const std::string& contact, int type) {
    return "register:code:" + contact + ":" + std::to_string(type);
}

std::string VerificationCodeService::getFrequencyKey(const std::string& contact) {
    return "register:frequency:" + contact;
}

} // namespace register_service
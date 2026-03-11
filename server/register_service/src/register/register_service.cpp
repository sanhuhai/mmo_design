#include "register/register_service.h"
#include "register/validation_service.h"
#include "register/verification_code_service.h"
#include <cstdlib>
#include <ctime>
#include <regex>

#ifdef ENABLE_REDIS
#include "redis/redis_client.h"
#endif

#ifdef ENABLE_DATABASE
#include "redis/database_client.h"
#endif

namespace register_service {

RegisterService* RegisterService::getInstance() {
    static RegisterService instance;
    return &instance;
}

RegisterService::RegisterService()
    : request_count_(0) {
    validation_service_ = std::make_unique<ValidationService>();
    verification_code_service_ = std::make_unique<VerificationCodeService>();
    srand(time(nullptr));
}

RegisterService::~RegisterService() {
}

bool RegisterService::initialize(const std::string& redis_host, int redis_port,
                               const std::string& db_host, int db_port,
                               const std::string& db_user, const std::string& db_password,
                               const std::string& db_name) {
    #ifdef ENABLE_REDIS
    redis_client_ = std::make_unique<redis::RedisClient>();
    if (!redis_client_->connect(redis_host, redis_port)) {
        last_error_ = "Failed to connect to Redis";
        return false;
    }
    #endif
    
    #ifdef ENABLE_DATABASE
    // 暂时注释掉数据库连接，因为 redis::DatabaseClient 类未定义
    // db_client_ = std::make_unique<redis::DatabaseClient>();
    // if (!db_client_->connect(db_host, db_port, db_user, db_password, db_name)) {
    //     last_error_ = "Failed to connect to database";
    //     return false;
    // }
    #endif
    
    #ifdef ENABLE_REDIS
    verification_code_service_->initialize(redis_client_);
    #else
    verification_code_service_->initialize();
    #endif
    
    return true;
}

#ifdef PROTOBUF_FOUND
int RegisterService::sendVerificationCode(const SendCodeRequest& request, SendCodeResponse& response) {
    std::string contact = request.phone().empty() ? request.email() : request.phone();
    int type = request.type();
    
    std::string error_msg;
    if (!request.phone().empty() && !validation_service_->validatePhone(contact, error_msg)) {
        last_error_ = error_msg;
        response.set_code(ERROR_INVALID_PHONE);
        response.set_message(error_msg);
        return ERROR_INVALID_PHONE;
    }
    
    if (!request.email().empty() && !validation_service_->validateEmail(contact, error_msg)) {
        last_error_ = error_msg;
        response.set_code(ERROR_INVALID_EMAIL);
        response.set_message(error_msg);
        return ERROR_INVALID_EMAIL;
    }
    
    if (!checkSendFrequency(contact, type)) {
        last_error_ = "Too many requests";
        response.set_code(ERROR_TOO_MANY_REQUESTS);
        response.set_message("Too many requests, please try again later");
        return ERROR_TOO_MANY_REQUESTS;
    }
    
    if (type == 1) { // 注册类型
        if (!request.phone().empty() && checkPhoneExists(request.phone())) {
            last_error_ = "Phone already registered";
            response.set_code(ERROR_PHONE_EXISTS);
            response.set_message("Phone already registered");
            return ERROR_PHONE_EXISTS;
        }
        if (!request.email().empty() && checkEmailExists(request.email())) {
            last_error_ = "Email already registered";
            response.set_code(ERROR_EMAIL_EXISTS);
            response.set_message("Email already registered");
            return ERROR_EMAIL_EXISTS;
        }
    }
    
    int error_code;
    std::string error_message;
    if (!verification_code_service_->sendCode(contact, type, error_code, error_message)) {
        last_error_ = error_message;
        response.set_code(error_code);
        response.set_message(error_message);
        return error_code;
    }
    
    response.set_code(SUCCESS);
    response.set_message("success");
    response.set_expire_time(300);
    response.set_retry_after(60);
    
    return SUCCESS;
}
#endif

int RegisterService::sendVerificationCode(const std::string& request, std::string& response) {
    // 简化实现，实际项目中需要解析请求字符串
    response = "{\"code\": 0, \"message\": \"success\", \"expire_time\": 300, \"retry_after\": 60}";
    return SUCCESS;
}

#ifdef PROTOBUF_FOUND
int RegisterService::registerAccount(const RegisterRequest& request, RegisterResponse& response) {
    std::string error_msg;
    if (!validation_service_->validateUsername(request.username(), error_msg)) {
        last_error_ = error_msg;
        response.set_code(ERROR_INVALID_USERNAME);
        response.set_message(error_msg);
        return ERROR_INVALID_USERNAME;
    }
    
    if (!validation_service_->validatePassword(request.password(), error_msg)) {
        last_error_ = error_msg;
        response.set_code(ERROR_INVALID_PASSWORD);
        response.set_message(error_msg);
        return ERROR_INVALID_PASSWORD;
    }
    
    if (request.password() != request.confirm_password()) {
        last_error_ = "Passwords do not match";
        response.set_code(ERROR_INVALID_PASSWORD);
        response.set_message("Passwords do not match");
        return ERROR_INVALID_PASSWORD;
    }
    
    if (!checkRegisterLimit(request.ip_address(), request.device_id())) {
        last_error_ = "Register limit exceeded";
        response.set_code(ERROR_REGISTER_LIMIT);
        response.set_message("Register limit exceeded, please try again tomorrow");
        return ERROR_REGISTER_LIMIT;
    }
    
    if (checkUsernameExists(request.username())) {
        last_error_ = "Username already exists";
        response.set_code(ERROR_USERNAME_EXISTS);
        response.set_message("Username already exists");
        return ERROR_USERNAME_EXISTS;
    }
    
    std::string contact = request.phone().empty() ? request.email() : request.phone();
    if (!verification_code_service_->verifyCode(contact, request.verification_code(), 1)) {
        last_error_ = "Invalid verification code";
        response.set_code(ERROR_INVALID_CODE);
        response.set_message("Invalid or expired verification code");
        return ERROR_INVALID_CODE;
    }
    
    int user_id = 0;
    int result = createUser(request, user_id);
    if (result != SUCCESS) {
        response.set_code(result);
        response.set_message(getLastError());
        return result;
    }
    
    verification_code_service_->deleteCode(contact, 1);
    
    RegisterReward reward;
    giveRegisterReward(user_id, reward);
    
    response.set_code(SUCCESS);
    response.set_message("success");
    response.set_user_id(user_id);
    response.set_username(request.username());
    response.set_token(generateToken(user_id));
    response.set_session_id("session_" + std::to_string(user_id));
    
    auto response_reward = response.mutable_reward();
    response_reward->set_gold(reward.gold());
    for (int item : reward.items()) {
        response_reward->add_items(item);
    }
    response_reward->set_experience(reward.experience());
    
    return SUCCESS;
}
#endif

int RegisterService::registerAccount(const std::string& request, std::string& response) {
    // 简化实现，实际项目中需要解析请求字符串
    int user_id = rand() % 100000 + 10000;
    response = "{\"code\": 0, \"message\": \"success\", \"user_id\": " + std::to_string(user_id) + ", \"username\": \"test_user\", \"token\": \"token_" + std::to_string(user_id) + "\", \"session_id\": \"session_" + std::to_string(user_id) + "\", \"reward\": {\"gold\": 1000, \"items\": [1001, 1002], \"experience\": 100}}";
    return SUCCESS;
}

#ifdef PROTOBUF_FOUND
int RegisterService::activateAccount(const ActivateRequest& request, ActivateResponse& response) {
    // 简化实现，实际项目中需要根据具体激活方式实现
    response.set_code(SUCCESS);
    response.set_message("success");
    response.set_activated(true);
    return SUCCESS;
}
#endif

int RegisterService::activateAccount(const std::string& request, std::string& response) {
    // 简化实现，实际项目中需要解析请求字符串
    response = "{\"code\": 0, \"message\": \"success\", \"activated\": true}";
    return SUCCESS;
}

bool RegisterService::validateUsername(const std::string& username) {
    std::string error_msg;
    return validation_service_->validateUsername(username, error_msg);
}

bool RegisterService::validatePassword(const std::string& password) {
    std::string error_msg;
    return validation_service_->validatePassword(password, error_msg);
}

bool RegisterService::validatePhone(const std::string& phone) {
    std::string error_msg;
    return validation_service_->validatePhone(phone, error_msg);
}

bool RegisterService::validateEmail(const std::string& email) {
    std::string error_msg;
    return validation_service_->validateEmail(email, error_msg);
}

bool RegisterService::checkUsernameExists(const std::string& username) {
    // 简化实现，实际项目中需要查询数据库
    return false;
}

bool RegisterService::checkPhoneExists(const std::string& phone) {
    // 简化实现，实际项目中需要查询数据库
    return false;
}

bool RegisterService::checkEmailExists(const std::string& email) {
    // 简化实现，实际项目中需要查询数据库
    return false;
}

bool RegisterService::checkSendFrequency(const std::string& contact, int type) {
    // 简化实现，实际项目中需要使用Redis进行频率限制
    return true;
}

bool RegisterService::checkRegisterLimit(const std::string& ip, const std::string& device_id) {
    // 简化实现，实际项目中需要使用Redis进行注册限制
    return true;
}

std::string RegisterService::generateVerificationCode() {
    static const char* chars = "0123456789";
    std::string code;
    for (int i = 0; i < 6; i++) {
        code += chars[rand() % 10];
    }
    return code;
}

std::string RegisterService::generateToken(int user_id) {
    // 简化实现，实际项目中需要使用JWT等方式生成token
    return "token_" + std::to_string(user_id) + "_" + std::to_string(time(nullptr));
}

std::string RegisterService::encryptPassword(const std::string& password) {
    // 简化实现，实际项目中需要使用bcrypt等算法加密
    return password;
}

#ifdef PROTOBUF_FOUND
int RegisterService::createUser(const RegisterRequest& request, int& user_id) {
    // 简化实现，实际项目中需要插入数据库
    user_id = rand() % 100000 + 10000;
    return SUCCESS;
}

bool RegisterService::giveRegisterReward(int user_id, RegisterReward& reward) {
    // 简化实现，实际项目中需要发放奖励
    reward.set_gold(1000);
    reward.add_items(1001);
    reward.add_items(1002);
    reward.set_experience(100);
    return true;
}
#endif

int RegisterService::createUser(const std::string& request, int& user_id) {
    // 简化实现，实际项目中需要解析请求字符串并插入数据库
    user_id = rand() % 100000 + 10000;
    return SUCCESS;
}

bool RegisterService::giveRegisterReward(int user_id, std::string& reward) {
    // 简化实现，实际项目中需要发放奖励
    reward = "{\"gold\": 1000, \"items\": [1001, 1002], \"experience\": 100}";
    return true;
}

std::string RegisterService::getLastError() const {
    return last_error_;
}

} // namespace register_service
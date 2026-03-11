#include "register/register_service.h"
#ifdef PROTOBUF_FOUND
#include "register.pb.h"
#endif
#include <iostream>

using namespace register_service;

int main() {
    // 初始化注册服务
    RegisterService* register_service = RegisterService::getInstance();
    if (!register_service->initialize("127.0.0.1", 6379, "localhost", 3306, "root", "password", "game_db")) {
        std::cerr << "Failed to initialize register service" << std::endl;
        return 1;
    }
    
    // 测试发送验证码
    std::cout << "=== Testing send verification code ===" << std::endl;
    #ifdef PROTOBUF_FOUND
    SendCodeRequest send_code_req;
    send_code_req.set_phone("13800138000");
    send_code_req.set_type(1); // 注册类型
    
    SendCodeResponse send_code_resp;
    int result = register_service->sendVerificationCode(send_code_req, send_code_resp);
    std::cout << "Send code result: " << result << std::endl;
    std::cout << "Message: " << send_code_resp.message() << std::endl;
    std::cout << "Expire time: " << send_code_resp.expire_time() << std::endl;
    std::cout << "Retry after: " << send_code_resp.retry_after() << std::endl;
    
    // 测试注册账号
    std::cout << "\n=== Testing register account ===" << std::endl;
    RegisterRequest register_req;
    register_req.set_username("test_user");
    register_req.set_password("Password123");
    register_req.set_confirm_password("Password123");
    register_req.set_phone("13800138000");
    register_req.set_verification_code("123456"); // 假设验证码是123456
    register_req.set_device_id("device_123");
    register_req.set_ip_address("192.168.1.1");
    
    RegisterResponse register_resp;
    result = register_service->registerAccount(register_req, register_resp);
    std::cout << "Register result: " << result << std::endl;
    std::cout << "Message: " << register_resp.message() << std::endl;
    if (result == SUCCESS) {
        std::cout << "User ID: " << register_resp.user_id() << std::endl;
        std::cout << "Username: " << register_resp.username() << std::endl;
        std::cout << "Token: " << register_resp.token() << std::endl;
        std::cout << "Session ID: " << register_resp.session_id() << std::endl;
        std::cout << "Reward - Gold: " << register_resp.reward().gold() << std::endl;
        std::cout << "Reward - Experience: " << register_resp.reward().experience() << std::endl;
        std::cout << "Reward - Items: ";
        for (int item : register_resp.reward().items()) {
            std::cout << item << " ";
        }
        std::cout << std::endl;
    }
    #else
    std::string send_code_req = "{\"phone\": \"13800138000\", \"type\": 1}";
    std::string send_code_resp;
    int result = register_service->sendVerificationCode(send_code_req, send_code_resp);
    std::cout << "Send code result: " << result << std::endl;
    std::cout << "Response: " << send_code_resp << std::endl;
    
    // 测试注册账号
    std::cout << "\n=== Testing register account ===" << std::endl;
    std::string register_req = "{\"username\": \"test_user\", \"password\": \"Password123\", \"confirm_password\": \"Password123\", \"phone\": \"13800138000\", \"verification_code\": \"123456\", \"device_id\": \"device_123\", \"ip_address\": \"192.168.1.1\"}";
    std::string register_resp;
    result = register_service->registerAccount(register_req, register_resp);
    std::cout << "Register result: " << result << std::endl;
    std::cout << "Response: " << register_resp << std::endl;
    #endif
    
    return 0;
}
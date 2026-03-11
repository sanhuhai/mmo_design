#include "register/message_handler.h"
#include <iostream>

#ifdef PROTOBUF_FOUND
#include "register.pb.h"
#endif

namespace register_service {

MessageHandler::MessageHandler() {
    register_service_ = RegisterService::getInstance();
}

MessageHandler::~MessageHandler() {
}

bool MessageHandler::initialize() {
    // 初始化注册服务
    if (!register_service_->initialize("127.0.0.1", 6379, "localhost", 3306, "root", "password", "game_db")) {
        std::cerr << "Failed to initialize register service" << std::endl;
        return false;
    }
    return true;
}

void MessageHandler::setResponseCallback(std::function<void(const std::string&)> callback) {
    response_callback_ = callback;
}

void MessageHandler::handleMessage(const std::string& message) {
    // 简化实现，实际项目中需要解析消息类型
    // 这里假设消息格式为：type|data
    size_t pos = message.find('|');
    if (pos == std::string::npos) {
        std::cerr << "Invalid message format" << std::endl;
        return;
    }
    
    std::string type = message.substr(0, pos);
    std::string data = message.substr(pos + 1);
    
    if (type == "send_code") {
        handleSendCodeRequest(data);
    } else if (type == "register") {
        handleRegisterRequest(data);
    } else if (type == "activate") {
        handleActivateRequest(data);
    } else {
        std::cerr << "Unknown message type: " << type << std::endl;
    }
}

#ifdef PROTOBUF_FOUND
void MessageHandler::handleSendCodeRequest(const std::string& message) {
    SendCodeRequest request;
    if (!request.ParseFromString(message)) {
        std::cerr << "Failed to parse send code request" << std::endl;
        return;
    }
    
    SendCodeResponse response;
    int result = register_service_->sendVerificationCode(request, response);
    
    std::string response_data;
    response.SerializeToString(&response_data);
    std::string response_message = "send_code_response|" + response_data;
    
    if (response_callback_) {
        response_callback_(response_message);
    }
}

void MessageHandler::handleRegisterRequest(const std::string& message) {
    RegisterRequest request;
    if (!request.ParseFromString(message)) {
        std::cerr << "Failed to parse register request" << std::endl;
        return;
    }
    
    RegisterResponse response;
    int result = register_service_->registerAccount(request, response);
    
    std::string response_data;
    response.SerializeToString(&response_data);
    std::string response_message = "register_response|" + response_data;
    
    if (response_callback_) {
        response_callback_(response_message);
    }
}

void MessageHandler::handleActivateRequest(const std::string& message) {
    ActivateRequest request;
    if (!request.ParseFromString(message)) {
        std::cerr << "Failed to parse activate request" << std::endl;
        return;
    }
    
    ActivateResponse response;
    int result = register_service_->activateAccount(request, response);
    
    std::string response_data;
    response.SerializeToString(&response_data);
    std::string response_message = "activate_response|" + response_data;
    
    if (response_callback_) {
        response_callback_(response_message);
    }
}
#endif

} // namespace register_service
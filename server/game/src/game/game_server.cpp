#include "game/game_server.h"
#include <iostream>

namespace game {

GameServer* GameServer::getInstance() {
    static GameServer instance;
    return &instance;
}

GameServer::GameServer() {
    user_manager_ = UserManager::getInstance();
}

GameServer::~GameServer() {
    stop();
}

bool GameServer::initialize(const std::string& host, int port) {
    // 设置回调函数
    server_.setConnectionCallback([this](uv_tcp_t* conn) {
        this->onConnection(conn);
    });
    
    server_.setMessageCallback([this](uv_tcp_t* conn, const std::string& message) {
        this->onMessage(conn, message);
    });
    
    server_.setErrorCallback([this](network::ErrorCode error) {
        this->onError(error);
    });
    
    // 启动服务器
    return server_.start(host, port);
}

void GameServer::start() {
    std::cout << "Game server started" << std::endl;
    // 运行事件循环
    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}

void GameServer::stop() {
    server_.stop();
    std::cout << "Game server stopped" << std::endl;
}

void GameServer::onConnection(uv_tcp_t* conn) {
    std::cout << "Client connected" << std::endl;
}

void GameServer::onMessage(uv_tcp_t* conn, const std::string& message) {
    // 解析消息类型
    // 这里简化处理，实际项目中应该根据消息头判断消息类型
    
    // 尝试解析为注册请求
    RegisterRequest register_request;
    if (register_request.ParseFromString(message)) {
        handleRegister(conn, register_request);
        return;
    }
    
    // 尝试解析为登录请求
    LoginRequest login_request;
    if (login_request.ParseFromString(message)) {
        handleLogin(conn, login_request);
        return;
    }
    
    // 尝试解析为心跳请求
    HeartbeatRequest heartbeat_request;
    if (heartbeat_request.ParseFromString(message)) {
        handleHeartbeat(conn, heartbeat_request);
        return;
    }
    
    std::cout << "Unknown message type" << std::endl;
}

void GameServer::onError(network::ErrorCode error) {
    std::cout << "Error: " << static_cast<int>(error) << std::endl;
}

void GameServer::handleRegister(uv_tcp_t* conn, const RegisterRequest& request) {
    std::cout << "Register request: " << request.username() << std::endl;
    
    int user_id;
    bool success = user_manager_->registerUser(
        request.username(),
        request.password(),
        request.email(),
        request.phone(),
        user_id
    );
    
    RegisterResponse response;
    if (success) {
        response.set_code(200);
        response.set_message("Register success");
        response.set_user_id(user_id);
    } else {
        response.set_code(400);
        response.set_message("Username already exists");
        response.set_user_id(0);
    }
    
    sendResponse(conn, response);
}

void GameServer::handleLogin(uv_tcp_t* conn, const LoginRequest& request) {
    std::cout << "Login request: " << request.username() << std::endl;
    
    User user;
    std::string token;
    bool success = user_manager_->loginUser(
        request.username(),
        request.password(),
        user,
        token
    );
    
    LoginResponse response;
    if (success) {
        response.set_code(200);
        response.set_message("Login success");
        response.set_user_id(user.user_id);
        response.set_username(user.username);
        response.set_token(token);
    } else {
        response.set_code(401);
        response.set_message("Invalid username or password");
        response.set_user_id(0);
    }
    
    sendResponse(conn, response);
}

void GameServer::handleHeartbeat(uv_tcp_t* conn, const HeartbeatRequest& request) {
    std::cout << "Heartbeat request: " << request.user_id() << std::endl;
    
    bool valid = user_manager_->validateToken(request.user_id(), request.token());
    
    HeartbeatResponse response;
    if (valid) {
        response.set_code(200);
        response.set_message("Heartbeat success");
        response.set_timestamp(time(nullptr));
    } else {
        response.set_code(401);
        response.set_message("Invalid token");
        response.set_timestamp(time(nullptr));
    }
    
    sendResponse(conn, response);
}

void GameServer::sendResponse(uv_tcp_t* conn, const google::protobuf::Message& response) {
    std::string response_str;
    if (response.SerializeToString(&response_str)) {
        server_.send(conn, response_str);
    } else {
        std::cerr << "Failed to serialize response" << std::endl;
    }
}

} // namespace game

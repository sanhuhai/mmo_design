#pragma once

#include "network/server.h"
#include "game/user_manager.h"
#include "game.pb.h"

namespace game {

class GameServer {
public:
    static GameServer* getInstance();
    
    bool initialize(const std::string& host, int port);
    void start();
    void stop();
    
private:
    GameServer();
    ~GameServer();
    
    network::Server server_;
    UserManager* user_manager_;
    
    void onConnection(uv_tcp_t* conn);
    void onMessage(uv_tcp_t* conn, const std::string& message);
    void onError(network::ErrorCode error);
    
    void handleRegister(uv_tcp_t* conn, const RegisterRequest& request);
    void handleLogin(uv_tcp_t* conn, const LoginRequest& request);
    void handleHeartbeat(uv_tcp_t* conn, const HeartbeatRequest& request);
    
    void sendResponse(uv_tcp_t* conn, const google::protobuf::Message& response);
};

} // namespace game

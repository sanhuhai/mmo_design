#include "game/game_server.h"
#include <iostream>

using namespace game;

int main() {
    // 初始化游戏服务器
    GameServer* game_server = GameServer::getInstance();
    if (!game_server->initialize("127.0.0.1", 8080)) {
        std::cerr << "Failed to initialize game server" << std::endl;
        return 1;
    }
    
    // 启动游戏服务器
    game_server->start();
    
    return 0;
}

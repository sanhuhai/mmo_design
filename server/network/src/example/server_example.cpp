#include "network/network.h"
#include "network/server.h"
#include <iostream>
#include <string>

using namespace network;

int main() {
    // 初始化网络库
    if (!init()) {
        std::cerr << "Failed to initialize network library" << std::endl;
        return 1;
    }

    // 创建服务器
    Server server;

    // 设置回调
    server.setConnectionCallback([](uv_tcp_t* conn) {
        std::cout << "New connection established" << std::endl;
    });

    server.setMessageCallback([&server](const std::string& message) {
        std::cout << "Received message: " << message << std::endl;
        // 回显消息
        // server.send(conn, "Echo: " + message);
    });

    server.setErrorCallback([](ErrorCode error) {
        std::cerr << "Error: " << static_cast<int>(error) << std::endl;
    });

    // 启动服务器
    if (!server.start("0.0.0.0", 8080)) {
        std::cerr << "Failed to start server" << std::endl;
        cleanup();
        return 1;
    }

    std::cout << "Server started. Press Enter to stop." << std::endl;
    std::cin.get();

    // 停止服务器
    server.stop();

    // 清理网络库
    cleanup();

    return 0;
}
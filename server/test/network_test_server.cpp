#include "network/server.h"
#include "test.pb.h"
#include <iostream>
#include <string>

using namespace network;
using namespace test;

int main() {
    Server server;
    
    // 设置连接回调
    server.setConnectionCallback([](uv_tcp_t* conn) {
        std::cout << "Client connected" << std::endl;
    });
    
    // 设置消息回调
    server.setMessageCallback([&server](uv_tcp_t* conn, const std::string& message) {
        // 解析 protobuf 消息
        std::cout << R"(Received message)" << std::endl;
        TestMessage test_message;
        if (test_message.ParseFromString(message)) {
            std::cout << "Received protobuf message: " << std::endl;
            std::cout << "  ID: " << test_message.id() << std::endl;
            std::cout << "  Content: " << test_message.content() << std::endl;
            std::cout << "  Timestamp: " << test_message.timestamp() << std::endl;
            
            // 构建响应消息
            TestResponse response;
            response.set_code(200);
            response.set_message("Success");
            *response.mutable_data() = test_message;
            
            // 序列化响应消息
            std::string response_str;
            response.SerializeToString(&response_str);
            
            // 回显消息给客户端
            if (!server.send(conn, response_str)) {
                std::cerr << "Failed to send response" << std::endl;
            } else {
                std::cout << "Response sent" << std::endl;
            }
        } else {
            std::cout << "Failed to parse protobuf message" << std::endl;
        }
    });
    
    // 设置错误回调
    server.setErrorCallback([](ErrorCode error) {
        std::cout << "Error: " << static_cast<int>(error) << std::endl;
    });
    
    // 启动服务器
    if (!server.start("127.0.0.1", 8080)) {
        std::cerr << "Failed to start server" << std::endl;
        return 1;
    }
    
    std::cout << "Server started on 127.0.0.1:8080" << std::endl;
    std::cout << "Press Ctrl+C to stop server" << std::endl;
    
    // 运行事件循环
    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
    
    return 0;
}

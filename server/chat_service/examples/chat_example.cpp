#include "chat/chat_server.h"
#include "chat/chat_message.h"
#include <iostream>

using namespace chat_service;

int main() {
    // 初始化聊天服务器
    ChatServer* chat_server = ChatServer::getInstance();
    if (!chat_server->initialize("127.0.0.1", 8080, "127.0.0.1", 6379, "localhost", 3306, "root", "password", "game_db")) {
        std::cerr << "Failed to initialize chat server" << std::endl;
        return 1;
    }
    
    // 启动聊天服务器
    chat_server->start();
    
    // 测试世界聊天
    std::cout << "=== Testing world chat ===" << std::endl;
    ChatMessage world_message;
#ifdef PROTOBUF_FOUND
    world_message.set_message_id(1);
    world_message.set_sender_id(1001);
    world_message.set_sender_name("Player1");
    world_message.set_type(MessageType::WORLD);
    world_message.set_content("Hello, world!");
    world_message.set_timestamp(time(nullptr));
    world_message.set_channel_id(0);
#else
    world_message.message_id = 1;
    world_message.sender_id = 1001;
    world_message.sender_name = "Player1";
    world_message.type = MessageType::WORLD;
    world_message.content = "Hello, world!";
    world_message.timestamp = time(nullptr);
    world_message.channel_id = 0;
#endif
    
    chat_server->handleMessage(1, world_message);
    
    // 测试公会聊天
    std::cout << "\n=== Testing guild chat ===" << std::endl;
    ChatMessage guild_message;
#ifdef PROTOBUF_FOUND
    guild_message.set_message_id(2);
    guild_message.set_sender_id(1001);
    guild_message.set_sender_name("Player1");
    guild_message.set_type(MessageType::GUILD);
    guild_message.set_content("Hello, guild!");
    guild_message.set_timestamp(time(nullptr));
    guild_message.set_channel_id(101); // 公会ID
#else
    guild_message.message_id = 2;
    guild_message.sender_id = 1001;
    guild_message.sender_name = "Player1";
    guild_message.type = MessageType::GUILD;
    guild_message.content = "Hello, guild!";
    guild_message.timestamp = time(nullptr);
    guild_message.channel_id = 101; // 公会ID
#endif
    
    chat_server->handleMessage(1, guild_message);
    
    // 测试私聊
    std::cout << "\n=== Testing private chat ===" << std::endl;
    ChatMessage private_message;
#ifdef PROTOBUF_FOUND
    private_message.set_message_id(3);
    private_message.set_sender_id(1001);
    private_message.set_sender_name("Player1");
    private_message.set_type(MessageType::PRIVATE);
    private_message.set_content("Hello, private!");
    private_message.set_timestamp(time(nullptr));
    private_message.set_target_id(1002); // 目标玩家ID
#else
    private_message.message_id = 3;
    private_message.sender_id = 1001;
    private_message.sender_name = "Player1";
    private_message.type = MessageType::PRIVATE;
    private_message.content = "Hello, private!";
    private_message.timestamp = time(nullptr);
    private_message.target_id = 1002; // 目标玩家ID
#endif
    
    chat_server->handleMessage(1, private_message);
    
    // 测试敏感词过滤
    std::cout << "\n=== Testing content filter ===" << std::endl;
    ChatMessage sensitive_message;
#ifdef PROTOBUF_FOUND
    sensitive_message.set_message_id(4);
    sensitive_message.set_sender_id(1001);
    sensitive_message.set_sender_name("Player1");
    sensitive_message.set_type(MessageType::WORLD);
    sensitive_message.set_content("This contains 敏感词1");
    sensitive_message.set_timestamp(time(nullptr));
    sensitive_message.set_channel_id(0);
#else
    sensitive_message.message_id = 4;
    sensitive_message.sender_id = 1001;
    sensitive_message.sender_name = "Player1";
    sensitive_message.type = MessageType::WORLD;
    sensitive_message.content = "This contains 敏感词1";
    sensitive_message.timestamp = time(nullptr);
    sensitive_message.channel_id = 0;
#endif
    
    chat_server->handleMessage(1, sensitive_message);
    
    // 停止聊天服务器
    chat_server->stop();
    
    return 0;
}

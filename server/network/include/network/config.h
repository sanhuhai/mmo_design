#ifndef CONFIG_H
#define CONFIG_H

namespace network {

struct Config {
    // 服务器配置
    std::string host = "0.0.0.0";
    int port = 8080;
    int max_connections = 10000;
    
    // 线程配置
    int io_threads = 4;
    int worker_threads = 8;
    
    // 心跳配置
    int heartbeat_interval = 30; // 秒
    int heartbeat_timeout = 60; // 秒
    
    // 会话配置
    int session_expire_time = 300; // 秒
    
    // 缓冲区配置
    int buffer_size = 8192;
    int max_packet_size = 1024 * 1024; // 1MB
};

} // namespace network

#endif // CONFIG_H
# C++ Game Server Network Module

一个基于LibUV的高性能C++游戏服务器网络模块，专为MMO游戏设计，提供完整的网络通信解决方案。

## 项目简介

本项目是一个轻量级、高性能的游戏服务器网络模块，采用异步I/O模型，支持高并发连接，适用于MMO、MOBA等需要处理大量客户端连接的游戏场景。

## 核心特性

- **异步I/O架构**：基于LibUV实现，支持跨平台（Windows/Linux/macOS）
- **高并发连接**：单线程可处理数千个并发连接
- **心跳机制**：自动检测和管理客户端连接状态
- **消息编解码**：支持长度前缀的消息格式，支持心跳消息
- **连接管理**：自动管理客户端连接生命周期
- **回调机制**：提供连接、消息、错误等事件回调接口
- **内存优化**：内置内存池和对象池，减少内存分配开销
- **多线程支持**：支持多线程IO和工作线程池
- **连接池管理**：高效的连接池复用机制

## 项目结构

```
network/
├── include/network/          # 头文件目录
│   ├── network.h           # 网络库主头文件
│   ├── server.h            # 服务器类定义
│   ├── connection.h        # 连接类定义
│   ├── message.h           # 消息处理类
│   ├── connection_pool.h   # 连接池
│   ├── memory_pool.h       # 内存池
│   ├── object_pool.h       # 对象池
│   ├── thread_pool.h       # 线程池
│   ├── multi_thread_server.h # 多线程服务器
│   ├── config.h            # 配置文件
│   ├── protobuf_message.h  # Protobuf消息支持
│   └── ...
├── src/network/            # 源文件目录
│   ├── network.cpp         # 网络库初始化
│   ├── server.cpp          # 服务器实现
│   ├── connection.cpp      # 连接管理实现
│   ├── message.cpp         # 消息处理实现
│   ├── connection_pool.cpp # 连接池实现
│   ├── memory_pool.cpp     # 内存池实现
│   ├── object_pool.cpp     # 对象池实现
│   ├── thread_pool.cpp     # 线程池实现
│   ├── multi_thread_server.cpp # 多线程服务器实现
│   ├── protobuf_message.cpp # Protobuf消息实现
│   └── ...
├── examples/               # 示例代码
│   └── server_example.cpp  # 服务器使用示例
├── third_party/            # 第三方依赖
│   └── libuv/             # LibUV网络库
├── proto/                  # Protobuf定义
│   └── game_message.proto  # 游戏消息定义
├── CMakeLists.txt         # CMake构建配置
└── README.md              # 项目说明文档
```

## 技术栈

- **C++11**：项目使用C++11标准
- **LibUV**：跨平台异步I/O库
- **CMake**：跨平台构建工具
- **Protobuf**：Google Protocol Buffers（可选）
- **ws2_32**：Windows网络库

## 环境要求

- CMake 3.10+
- C++11兼容的编译器（GCC 4.8+, Clang 3.3+, MSVC 2015+）
- LibUV 1.x

## 构建说明

### 1. 下载并构建LibUV

```bash
cd third_party
git clone https://github.com/libuv/libuv.git
cd libuv
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

### 2. 构建项目

```bash
cd network
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

### 3. 运行示例

```bash
./Release/server_example.exe
```

## 使用示例

### 基本服务器使用

```cpp
#include "network/server.h"
#include <iostream>

int main() {
    // 初始化网络库
    network::init();
    
    // 创建服务器
    network::Server server;
    
    // 设置回调函数
    server.setConnectionCallback([](uv_tcp_t* conn) {
        std::cout << "New client connected" << std::endl;
    });
    
    server.setMessageCallback([](const std::string& message) {
        std::cout << "Received message: " << message << std::endl;
    });
    
    server.setErrorCallback([](network::ErrorCode error) {
        std::cerr << "Error occurred: " << static_cast<int>(error) << std::endl;
    });
    
    // 启动服务器
    if (server.start("0.0.0.0", 8888)) {
        std::cout << "Server is running..." << std::endl;
        // 运行事件循环
        uv_run(uv_default_loop(), UV_RUN_DEFAULT);
    }
    
    // 清理网络库
    network::cleanup();
    
    return 0;
}
```

## 核心组件

### Server（服务器）

服务器类是网络模块的核心，负责：
- 监听客户端连接
- 管理所有客户端连接
- 处理网络事件
- 提供回调接口

主要方法：
- `start(host, port)` - 启动服务器
- `stop()` - 停止服务器
- `send(conn, message)` - 发送消息
- `close(conn)` - 关闭连接

### Connection（连接）

连接类管理单个客户端连接：
- 维护客户端地址信息
- 跟踪心跳时间
- 提供连接状态检查

主要方法：
- `getClientAddress()` - 获取客户端地址
- `checkHeartbeat()` - 检查心跳状态
- `updateHeartbeat()` - 更新心跳时间

### Message（消息）

消息类提供消息编解码功能：
- 长度前缀编码/解码
- 心跳消息创建和检测
- 消息格式验证

主要方法：
- `encode(message)` - 编码消息
- `decode(data, consumed)` - 解码消息
- `createHeartbeatMessage()` - 创建心跳消息
- `isHeartbeatMessage(message)` - 检查是否为心跳消息

## 性能优化

### 内存池

内存池通过预分配内存块，减少频繁的内存分配和释放操作，提高性能。

### 对象池

对象池复用对象实例，避免重复创建和销毁对象，减少GC压力。

### 连接池

连接池管理数据库连接等资源，提高资源利用率。

### 多线程IO

支持多线程IO模型，充分利用多核CPU性能。

## 消息协议

### 消息格式

```
+--------+--------+--------+--------+------------------+
| Length (4 bytes)                | Message Body     |
+--------+--------+--------+--------+------------------+
```

- Length：消息体长度（4字节，网络字节序）
- Message Body：实际消息内容

### 心跳机制

- 客户端定期发送心跳消息
- 服务器检测心跳超时
- 自动清理无效连接

## 错误处理

网络模块定义了以下错误码：

```cpp
enum class ErrorCode {
    OK = 0,                // 成功
    INVALID_PARAM = 1,     // 参数错误
    NETWORK_ERROR = 2,     // 网络错误
    INTERNAL_ERROR = 3     // 内部错误
};
```

## 配置选项

通过修改 `config.h` 可以配置以下参数：

- 最大连接数
- 心跳超时时间
- 缓冲区大小
- 线程池大小

## 扩展功能

### Protobuf支持

项目支持使用Protobuf进行消息序列化，提供更高效的消息编码。

### 多线程服务器

`MultiThreadServer` 类提供多线程服务器实现，支持更高的并发性能。

## 注意事项

1. 确保LibUV已正确编译并链接
2. Windows下需要链接ws2_32库
3. 生产环境建议使用Release配置
4. 注意内存泄漏和线程安全问题

## 未来计划

- [ ] 添加KCP支持（可靠UDP）
- [ ] 实现SSL/TLS加密
- [ ] 添加更多协议支持
- [ ] 性能测试和优化
- [ ] 完善文档和示例

## 许可证

本项目采用MIT许可证。

## 贡献

欢迎提交Issue和Pull Request！

## 联系方式

如有问题或建议，请通过以下方式联系：

- 提交Issue
- 发送邮件
- 加入讨论组

## 致谢

感谢以下开源项目的支持：

- [LibUV](https://github.com/libuv/libuv) - 异步I/O库
- [Protobuf](https://github.com/protocolbuffers/protobuf) - Protocol Buffers
- [CMake](https://cmake.org/) - 跨平台构建工具

---

**注意**：本项目仍在开发中，部分功能可能尚未完善。使用时请注意测试和验证。

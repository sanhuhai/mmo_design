# C++游戏服务器框架

这是一个基于C++11的游戏服务器框架，包含网络模块和角色系统模块，为游戏服务器开发提供基础架构。

## 项目结构

```
server/
├── CMakeLists.txt         # 根CMake配置文件
├── README.md              # 项目说明文档
├── network/               # 网络模块
│   ├── CMakeLists.txt     # 网络模块CMake配置
│   ├── include/           # 头文件
│   ├── src/               # 源代码
│   └── examples/          # 示例程序
├── character_system/      # 角色系统模块
│   ├── CMakeLists.txt     # 角色系统CMake配置
│   ├── include/           # 头文件
│   ├── src/               # 源代码
│   └── examples/          # 示例程序
└── third_party/           # 第三方依赖
```

## 功能特点

### 网络模块
- 基于LibUV的高性能网络库
- TCP服务器实现
- 连接管理与心跳机制
- 消息编解码系统
- 跨平台支持（Windows/Linux）

### 角色系统模块
- 基于ECS（Entity-Component-System）架构
- 基础角色类（Character）
- 玩家类（Player）- 支持升级系统
- NPC类（NPC）- 支持AI行为
- 通用组件基类（Component）
  - 生命周期管理
  - 事件系统
  - 状态控制
  - 优先级系统
  - 依赖管理
- 专用组件
  - 战斗组件（CombatComponent）
  - 移动组件（MovementComponent）
  - 网络同步组件（NetworkSyncComponent）
- 角色管理器（CharacterManager）- 单例模式

## 技术栈

- C++11
- CMake 3.10+
- LibUV 1.x (网络库)
- 跨平台支持（Windows/Linux）

## 安装与构建

### 前提条件

- CMake 3.10或更高版本
- C++11兼容编译器
- LibUV库（可选，用于网络模块）

### 安装LibUV

#### Windows
```bash
git clone https://github.com/libuv/libuv.git third_party/libuv
cd third_party/libuv
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

#### Linux
```bash
git clone https://github.com/libuv/libuv.git third_party/libuv
cd third_party/libuv
mkdir build && cd build
cmake ..
make
```

或者使用包管理器安装：
```bash
# Ubuntu/Debian
sudo apt-get install libuv1-dev

# CentOS/RHEL
sudo yum install libuv-devel
```

### 构建项目

#### Windows
```bash
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

#### Linux
```bash
mkdir build && cd build
cmake ..
make
```

## 运行示例

### 网络服务器示例
```bash
# Windows
build/network/Release/server_example.exe

# Linux
build/network/server_example
```

### 角色系统示例
```bash
# Windows
build/character_system/Release/component_test.exe

# Linux
build/character_system/component_test
```

## 开发指南

### 网络模块使用

```cpp
#include "network/network.h"
#include "network/server.h"

// 初始化网络库
if (!init()) {
    // 错误处理
    return 1;
}

// 创建服务器
Server server;

// 设置回调
server.setConnectionCallback([](uv_tcp_t* conn) {
    // 新连接处理
});

server.setMessageCallback([](const std::string& message) {
    // 消息处理
});

// 启动服务器
if (!server.start("0.0.0.0", 8080)) {
    // 错误处理
    return 1;
}

// 停止服务器
server.stop();

// 清理
cleanup();
```

### 角色系统使用

```cpp
#include "character/character_manager.h"
#include "character/combat_component.h"

// 获取角色管理器实例
CharacterManager& manager = CharacterManager::getInstance();
manager.initialize();

// 创建玩家
Player* player = manager.createPlayer(1001, "Hero", 10001);

// 添加组件
auto combat = std::make_unique<CombatComponent>(player);
combat->setHealth(100);
combat->setDamage(20);
player->addComponent(std::move(combat));

// 更新角色
manager.update(0.016f); // 60 FPS

// 移除角色
manager.removeCharacter(1001);
```

## 配置选项

CMake构建系统支持以下选项：

| 选项 | 描述 | 默认值 |
|------|------|--------|
| BUILD_NETWORK_MODULE | 构建网络模块 | ON |
| BUILD_CHARACTER_SYSTEM | 构建角色系统 | ON |
| BUILD_TESTS | 构建测试 | OFF |

例如：
```bash
cmake .. -DBUILD_NETWORK_MODULE=ON -DBUILD_CHARACTER_SYSTEM=ON
```

## 平台注意事项

### Windows
- 需要Winsock2库（ws2_32）
- 建议使用Visual Studio 2019或更高版本

### Linux
- 需要pthread库
- 建议使用GCC 7+或Clang 5+

## 性能优化

- 网络模块使用LibUV的异步IO模型
- 角色系统采用组件化设计，支持按需加载
- 连接池管理减少内存分配
- 心跳机制保持连接活跃

## 扩展建议

1. **添加数据库模块** - 用于持久化角色数据
2. **实现脚本系统** - 支持Lua或Python脚本
3. **添加日志系统** - 用于调试和监控
4. **实现配置系统** - 支持运行时配置
5. **添加安全模块** - 防止恶意攻击

## 许可证

本项目采用MIT许可证，详见LICENSE文件。

## 联系方式

如有问题或建议，请联系项目维护者。

---

*项目创建日期：2026-03-11*
*版本：1.0.0*

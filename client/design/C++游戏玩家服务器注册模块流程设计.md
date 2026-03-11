# 大型C++ MMO游戏玩家服务器注册模块流程设计

## 1. 系统架构

### 整体架构
```
┌─────────────────────────────────────────────────────────────────┐
│                         客户端                                   │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐             │
│  │ 注册界面    │  │ 验证码输入  │  │ 账号激活    │             │
│  └──────┬──────┘  └──────┬──────┘  └──────┬──────┘             │
└─────────┼────────────────┼────────────────┼─────────────────────┘
          │                │                │
          ▼                ▼                ▼
┌─────────────────────────────────────────────────────────────────┐
│                        API网关                                  │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐             │
│  │ 负载均衡    │  │ 限流控制    │  │ 安全验证    │             │
│  └──────┬──────┘  └──────┬──────┘  └──────┬──────┘             │
└─────────┼────────────────┼────────────────┼─────────────────────┘
          │                │                │
          ▼                ▼                ▼
┌─────────────────────────────────────────────────────────────────┐
│                      注册服务集群                                │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │                    注册服务                               │    │
│  │  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌────────┐  │    │
│  │  │ 数据验证 │  │ 验证码   │  │ 账号创建 │  │ 日志   │  │    │
│  │  │ 服务     │  │ 服务     │  │ 服务     │  │ 服务   │  │    │
│  │  └──────────┘  └──────────┘  └──────────┘  └────────┘  │    │
│  └─────────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────────┘
          │                │                │
          ▼                ▼                ▼
┌─────────────────────────────────────────────────────────────────┐
│                        中间件                                   │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐             │
│  │ Redis缓存   │  │ 消息队列    │  │ 短信/邮件  │             │
│  └─────────────┘  └─────────────┘  └─────────────┘             │
└─────────────────────────────────────────────────────────────────┘
          │                │                │
          ▼                ▼                ▼
┌─────────────────────────────────────────────────────────────────┐
│                        数据存储层                               │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐             │
│  │ MySQL主库   │  │ MySQL从库   │  │ 日志存储    │             │
│  └─────────────┘  └─────────────┘  └─────────────┘             │
└─────────────────────────────────────────────────────────────────┘
```

### 模块职责

#### API网关
- **负载均衡**：将请求分发到多个注册服务实例
- **限流控制**：防止恶意注册请求
- **安全验证**：验证码、IP过滤等
- **协议转换**：HTTP/JSON到内部协议转换

#### 注册服务
- **数据验证**：验证用户输入的合法性
- **验证码服务**：生成、发送、验证验证码
- **账号创建**：创建用户账号
- **日志服务**：记录注册过程

#### 缓存层
- **验证码缓存**：存储验证码信息
- **限流缓存**：存储限流计数器
- **热点数据**：缓存热门账号信息

#### 消息队列
- **异步任务**：验证码发送等异步任务
- **流量削峰**：缓解高峰期压力
- **任务重试**：失败任务重试

## 2. 注册流程设计

### 完整流程图
```
┌────────┐     ┌────────┐     ┌────────┐     ┌────────┐     ┌────────┐
│ 客户端 │────▶│ API    │────▶│ 注册   │────▶│ 验证   │────▶│ 创建   │
│ 提交   │     │ 网关   │     │ 服务   │     │ 码服务 │     │ 账号   │
└────────┘     └────────┘     └────────┘     └────────┘     └────────┘
                                                                  │
                         ┌────────────────────────────────────────┘
                         ▼
┌────────┐     ┌────────┐     ┌────────┐     ┌────────┐     ┌────────┐
│ 客户端 │◀────│ API    │◀────│ 注册   │◀────│ 奖励   │◀────│ 激活   │
│ 完成   │     │ 网关   │     │ 服务   │     │ 发放   │     │ 账号   │
└────────┘     └────────┘     └────────┘     └────────┘     └────────┘
```

### 详细流程

#### 流程一：发送验证码
```
1. 客户端发送验证码请求
   POST /api/v1/register/send_code
   {
       "phone": "13800138000",
       "type": 1  // 1-注册 2-找回密码 3-绑定手机
   }

2. API网关验证请求
   - 检查请求格式
   - 验证IP地址
   - 检查频率限制

3. 注册服务处理
   a) 验证手机号格式
   b) 检查手机号是否已注册（注册类型）
   c) 检查发送频率限制
   d) 生成6位随机验证码
   e) 存储验证码到Redis（有效期5分钟）
   f) 发送验证码到短信平台

4. 返回结果
   {
       "code": 0,
       "message": "success",
       "data": {
           "expire_time": 300,
           "retry_after": 60
       }
   }
```

#### 流程二：注册账号
```
1. 客户端提交注册请求
   POST /api/v1/register
   {
       "username": "player001",
       "password": "Password123",
       "confirm_password": "Password123",
       "phone": "13800138000",
       "verification_code": "123456",
       "invite_code": "ABC123",  // 可选
       "device_id": "device_xxx",
       "ip_address": "192.168.1.1"
   }

2. API网关验证请求
   - 检查请求格式
   - 验证IP地址
   - 检查频率限制
   - 验证验证码

3. 注册服务处理
   a) 验证用户名
      - 长度4-20字符
      - 只允许字母、数字、下划线
      - 不能是敏感词
      - 不能已存在
   
   b) 验证密码
      - 长度8-32字符
      - 必须包含字母和数字
      - 复杂度检查
   
   c) 验证验证码
      - 从Redis获取验证码
      - 验证是否正确
      - 验证是否过期
      - 验证成功后删除
   
   d) 检查设备注册限制
      - 同一设备每天最多注册2个
      - 同一IP每天最多注册10个
   
   e) 加密密码
      - 使用bcrypt加密
      - 生成随机盐值
   
   f) 创建账号
      - 生成唯一用户ID
      - 插入用户基础信息
      - 初始化角色数据
      - 记录注册日志
   
   g) 发放注册奖励
      - 新手礼包
      - 绑定金币
      - 初始道具
   
   h) 创建游戏角色（可选）

4. 返回结果
   {
       "code": 0,
       "message": "success",
       "data": {
           "user_id": 10001,
           "username": "player001",
           "token": "xxx",
           "reward": {
               "gold": 1000,
               "items": [1001, 1002]
           }
       }
   }
```

#### 流程三：账号激活（可选）
```
1. 邮箱激活流程
   a) 用户注册时填写邮箱
   b) 发送激活邮件到用户邮箱
   c) 用户点击激活链接
   d) 服务验证激活码
   e) 激活账号

2. 短信激活流程
   a) 用户注册时填写手机号
   b) 发送激活短信
   c) 用户输入激活码
   d) 服务验证激活码
   e) 激活账号
```

## 3. 核心数据结构

### 用户基础信息
```cpp
struct UserBaseInfo {
    int user_id;                // 用户唯一ID
    std::string username;       // 用户名
    std::string password_hash;  // 密码哈希
    std::string phone;          // 手机号
    std::string email;          // 邮箱
    int status;                 // 账号状态：0-未激活，1-已激活，2-已封禁
    int register_source;        // 注册来源：1-手机，2-邮箱，3-第三方
    std::string register_ip;    // 注册IP
    std::string device_id;      // 设备ID
    long create_time;           // 创建时间
    long last_login_time;       // 最后登录时间
    long last_logout_time;      // 最后登出时间
};
```

### 验证码信息
```cpp
struct VerificationCode {
    std::string code;           // 验证码
    std::string phone;           // 手机号/邮箱
    int type;                   // 验证码类型
    long create_time;           // 创建时间
    long expire_time;           // 过期时间
    int send_count;             // 发送次数
};
```

### 注册请求
```cpp
struct RegisterRequest {
    std::string username;       // 用户名
    std::string password;       // 密码
    std::string confirm_password; // 确认密码
    std::string phone;          // 手机号
    std::string email;          // 邮箱
    std::string verification_code; // 验证码
    std::string invite_code;   // 邀请码
    std::string device_id;      // 设备ID
    std::string device_type;    // 设备类型
    std::string ip_address;     // IP地址
    std::string mac_address;    // MAC地址
    std::string client_version; // 客户端版本
};
```

### 注册响应
```cpp
struct RegisterResponse {
    int user_id;                // 用户ID
    std::string username;       // 用户名
    std::string token;          // 登录令牌
    std::string session_id;     // 会话ID
    RegisterReward reward;      // 注册奖励
};

struct RegisterReward {
    int gold;                   // 金币
    std::vector<int> items;     // 道具ID列表
    int experience;             // 经验
};
```

## 4. 核心服务实现

### 注册服务主类
```cpp
#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <atomic>

#include "redis_client.h"
#include "database_client.h"
#include "sms_client.h"
#include "email_client.h"
#include "logger.h"
#include "proto/register.pb.h"

class RegisterService {
public:
    static RegisterService* getInstance();
    
    bool initialize(const std::string& redis_host, int redis_port,
                   const std::string& db_host, int db_port,
                   const std::string& db_user, const std::string& db_password,
                   const std::string& db_name);
    
    int sendVerificationCode(const SendCodeRequest& request, SendCodeResponse& response);
    int registerAccount(const RegisterRequest& request, RegisterResponse& response);
    int activateAccount(const ActivateRequest& request, ActivateResponse& response);
    
private:
    RegisterService();
    ~RegisterService();
    
    bool validateUsername(const std::string& username);
    bool validatePassword(const std::string& password);
    bool validatePhone(const std::string& phone);
    bool validateEmail(const std::string& email);
    
    bool checkUsernameExists(const std::string& username);
    bool checkPhoneExists(const std::string& phone);
    bool checkEmailExists(const std::string& email);
    
    bool checkSendFrequency(const std::string& contact, int type);
    bool checkRegisterLimit(const std::string& ip, const std::string& device_id);
    
    std::string generateVerificationCode();
    std::string generateUserId();
    std::string encryptPassword(const std::string& password);
    
    bool saveVerificationCode(const VerificationCode& code);
    bool verifyCode(const std::string& contact, const std::string& code, int type);
    bool deleteCode(const std::string& contact, int type);
    
    int createUser(const RegisterRequest& request, int& user_id);
    bool giveRegisterReward(int user_id, RegisterReward& reward);
    
    std::string getLastError() const;
    
private:
    std::unique_ptr<RedisClient> redis_client_;
    std::unique_ptr<DatabaseClient> db_client_;
    std::unique_ptr<SmsClient> sms_client_;
    std::unique_ptr<EmailClient> email_client_;
    std::unique_ptr<Logger> logger_;
    
    std::mutex register_mutex_;
    std::atomic<int> request_count_;
    
    std::unordered_map<std::string, std::string> config_;
    std::string last_error_;
};
```

### 验证码服务
```cpp
class VerificationCodeService {
public:
    bool sendCode(const std::string& contact, int type, int& error_code, std::string& error_msg);
    bool verifyCode(const std::string& contact, const std::string& code, int type);
    bool deleteCode(const std::string& contact, int type);
    
private:
    std::string generateCode();
    bool saveToCache(const VerificationCode& code);
    std::string getCacheKey(const std::string& contact, int type);
    
    static const int CODE_LENGTH = 6;
    static const int CODE_EXPIRE_TIME = 300;  // 5分钟
    static const int MAX_SEND_COUNT_PER_DAY = 10;
    static const int MIN_SEND_INTERVAL = 60;  // 60秒
};
```

### 数据验证服务
```cpp
class ValidationService {
public:
    bool validateUsername(const std::string& username, std::string& error_msg);
    bool validatePassword(const std::string& password, std::string& error_msg);
    bool validatePhone(const std::string& phone, std::string& error_msg);
    bool validateEmail(const std::string& email, std::string& error_msg);
    
private:
    bool isValidUsernameFormat(const std::string& username);
    bool isValidPasswordFormat(const std::string& password);
    bool isValidPhoneFormat(const std::string& phone);
    bool isValidEmailFormat(const std::string& email);
    bool isSensitiveWord(const std::string& text);
    
    static const int USERNAME_MIN_LENGTH = 4;
    static const int USERNAME_MAX_LENGTH = 20;
    static const int PASSWORD_MIN_LENGTH = 8;
    static const int PASSWORD_MAX_LENGTH = 32;
};
```

### 注册服务实现
```cpp
#include "register_service.h"

RegisterService* RegisterService::getInstance() {
    static RegisterService instance;
    return &instance;
}

int RegisterService::sendVerificationCode(const SendCodeRequest& request, SendCodeResponse& response) {
    std::string contact = request.phone().empty() ? request.email() : request.phone();
    int type = request.type();
    
    if (!validatePhone(contact) && !validateEmail(contact)) {
        last_error_ = "Invalid contact format";
        return ERROR_INVALID_CONTACT;
    }
    
    if (!checkSendFrequency(contact, type)) {
        last_error_ = "Too many requests";
        return ERROR_TOO_MANY_REQUESTS;
    }
    
    if (type == TYPE_REGISTER) {
        if (!request.phone().empty() && checkPhoneExists(request.phone())) {
            last_error_ = "Phone already registered";
            return ERROR_PHONE_EXISTS;
        }
        if (!request.email().empty() && checkEmailExists(request.email())) {
            last_error_ = "Email already registered";
            return ERROR_EMAIL_EXISTS;
        }
    }
    
    std::string code = generateVerificationCode();
    
    VerificationCode verification_code;
    verification_code.code = code;
    verification_code.phone = contact;
    verification_code.type = type;
    verification_code.create_time = time(nullptr);
    verification_code.expire_time = verification_code.create_time + 300;
    verification_code.send_count = 1;
    
    if (!saveVerificationCode(verification_code)) {
        last_error_ = "Failed to save verification code";
        return ERROR_INTERNAL;
    }
    
    bool sent = false;
    if (!request.phone().empty()) {
        sent = sms_client_->send(contact, code);
    } else {
        sent = email_client_->send(contact, "注册验证码", "您的验证码是：" + code);
    }
    
    if (!sent) {
        last_error_ = "Failed to send verification code";
        return ERROR_SEND_FAILED;
    }
    
    response.set_expire_time(300);
    response.set_retry_after(60);
    
    return SUCCESS;
}

int RegisterService::registerAccount(const RegisterRequest& request, RegisterResponse& response) {
    if (!validateUsername(request.username())) {
        last_error_ = "Invalid username format";
        return ERROR_INVALID_USERNAME;
    }
    
    if (!validatePassword(request.password())) {
        last_error_ = "Invalid password format";
        return ERROR_INVALID_PASSWORD;
    }
    
    if (!checkRegisterLimit(request.ip_address(), request.device_id())) {
        last_error_ = "Register limit exceeded";
        return ERROR_REGISTER_LIMIT;
    }
    
    if (checkUsernameExists(request.username())) {
        last_error_ = "Username already exists";
        return ERROR_USERNAME_EXISTS;
    }
    
    if (!verifyCode(request.phone().empty() ? request.email() : request.phone(), 
                   request.verification_code(), TYPE_REGISTER)) {
        last_error_ = "Invalid verification code";
        return ERROR_INVALID_CODE;
    }
    
    std::string password_hash = encryptPassword(request.password());
    
    int user_id = 0;
    int result = createUser(request, user_id);
    if (result != SUCCESS) {
        return result;
    }
    
    deleteCode(request.phone().empty() ? request.email() : request.phone(), TYPE_REGISTER);
    
    giveRegisterReward(user_id, response.mutable_reward());
    
    response.set_user_id(user_id);
    response.set_username(request.username());
    response.set_token(generateToken(user_id));
    
    logger_->info("User registered: user_id=%d, username=%s, ip=%s", 
                  user_id, request.username().c_str(), request.ip_address().c_str());
    
    return SUCCESS;
}

std::string RegisterService::encryptPassword(const std::string& password) {
    std::string salt = generateSalt(16);
    std::string hash = bcrypt_hash(password, salt);
    return salt + "$" + hash;
}

std::string RegisterService::generateVerificationCode() {
    static const char* chars = "0123456789";
    std::string code;
    for (int i = 0; i < 6; i++) {
        code += chars[rand() % 10];
    }
    return code;
}
```

## 5. 数据库设计

### 用户表
```sql
CREATE TABLE `users` (
    `user_id` INT NOT NULL AUTO_INCREMENT COMMENT '用户ID',
    `username` VARCHAR(50) NOT NULL COMMENT '用户名',
    `password_hash` VARCHAR(255) NOT NULL COMMENT '密码哈希',
    `phone` VARCHAR(20) DEFAULT NULL COMMENT '手机号',
    `email` VARCHAR(100) DEFAULT NULL COMMENT '邮箱',
    `status` TINYINT NOT NULL DEFAULT 0 COMMENT '账号状态：0-未激活，1-已激活，2-已封禁',
    `register_source` TINYINT NOT NULL DEFAULT 1 COMMENT '注册来源：1-手机，2-邮箱，3-第三方',
    `register_ip` VARCHAR(50) DEFAULT NULL COMMENT '注册IP',
    `device_id` VARCHAR(100) DEFAULT NULL COMMENT '设备ID',
    `create_time` BIGINT NOT NULL COMMENT '创建时间',
    `last_login_time` BIGINT DEFAULT NULL COMMENT '最后登录时间',
    `last_logout_time` BIGINT DEFAULT NULL COMMENT '最后登出时间',
    PRIMARY KEY (`user_id`),
    UNIQUE KEY `uk_username` (`username`),
    UNIQUE KEY `uk_phone` (`phone`),
    UNIQUE KEY `uk_email` (`email`),
    KEY `idx_create_time` (`create_time`),
    KEY `idx_status` (`status`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='用户表';
```

### 验证码表
```sql
CREATE TABLE `verification_codes` (
    `id` INT NOT NULL AUTO_INCREMENT COMMENT 'ID',
    `contact` VARCHAR(100) NOT NULL COMMENT '联系方式',
    `code` VARCHAR(20) NOT NULL COMMENT '验证码',
    `type` TINYINT NOT NULL COMMENT '类型：1-注册，2-找回密码，3-绑定手机',
    `expire_time` BIGINT NOT NULL COMMENT '过期时间',
    `create_time` BIGINT NOT NULL COMMENT '创建时间',
    `used_time` BIGINT DEFAULT NULL COMMENT '使用时间',
    PRIMARY KEY (`id`),
    KEY `idx_contact_type` (`contact`, `type`),
    KEY `idx_expire_time` (`expire_time`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='验证码表';
```

### 注册日志表
```sql
CREATE TABLE `register_logs` (
    `id` BIGINT NOT NULL AUTO_INCREMENT COMMENT 'ID',
    `user_id` INT DEFAULT NULL COMMENT '用户ID',
    `username` VARCHAR(50) DEFAULT NULL COMMENT '用户名',
    `phone` VARCHAR(20) DEFAULT NULL COMMENT '手机号',
    `email` VARCHAR(100) DEFAULT NULL COMMENT '邮箱',
    `register_ip` VARCHAR(50) DEFAULT NULL COMMENT '注册IP',
    `device_id` VARCHAR(100) DEFAULT NULL COMMENT '设备ID',
    `device_type` VARCHAR(50) DEFAULT NULL COMMENT '设备类型',
    `client_version` VARCHAR(50) DEFAULT NULL COMMENT '客户端版本',
    `status` TINYINT NOT NULL DEFAULT 0 COMMENT '状态：0-失败，1-成功',
    `error_message` VARCHAR(500) DEFAULT NULL COMMENT '错误信息',
    `create_time` BIGINT NOT NULL COMMENT '创建时间',
    PRIMARY KEY (`id`),
    KEY `idx_user_id` (`user_id`),
    KEY `idx_create_time` (`create_time`),
    KEY `idx_register_ip` (`register_ip`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='注册日志表';
```

### 注册限制表
```sql
CREATE TABLE `register_limits` (
    `id` BIGINT NOT NULL AUTO_INCREMENT COMMENT 'ID',
    `ip_address` VARCHAR(50) NOT NULL COMMENT 'IP地址',
    `device_id` VARCHAR(100) NOT NULL COMMENT '设备ID',
    `register_count` INT NOT NULL DEFAULT 1 COMMENT '注册次数',
    `first_register_time` BIGINT NOT NULL COMMENT '首次注册时间',
    `last_register_time` BIGINT NOT NULL COMMENT '最后注册时间',
    PRIMARY KEY (`id`),
    UNIQUE KEY `uk_ip_device` (`ip_address`, `device_id`),
    KEY `idx_ip_address` (`ip_address`),
    KEY `idx_device_id` (`device_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='注册限制表';
```

## 6. Redis缓存设计

### Key设计
```
注册验证码：register:code:{phone}:{type} -> 验证码
发送频率限制：register:frequency:{phone} -> 发送次数
IP注册限制：register:limit:ip:{ip} -> 注册次数
设备注册限制：register:limit:device:{device_id} -> 注册次数
注册锁：register:lock:{ip}:{device_id} -> 锁状态
```

### 过期时间
```
验证码：300秒（5分钟）
发送频率：3600秒（1小时）
IP注册限制：86400秒（24小时）
设备注册限制：86400秒（24小时）
注册锁：30秒
```

## 7. 安全措施

### 防刷注册
- **IP限制**：同一IP每天最多注册10个账号
- **设备限制**：同一设备每天最多注册2个账号
- **频率限制**：同一手机号每60秒只能发送一次验证码
- **验证码限制**：每天最多发送10次验证码

### 密码安全
- **bcrypt加密**：使用bcrypt算法加密密码
- **随机盐值**：每个密码使用随机盐值
- **密码复杂度**：强制要求密码包含字母和数字

### 数据安全
- **敏感数据加密**：密码、支付信息等敏感数据加密存储
- **日志脱敏**：日志中的敏感信息进行脱敏处理
- **数据传输**：使用HTTPS加密传输

### 异常检测
- **异常注册检测**：检测异常注册行为
- **批量注册检测**：检测批量注册行为
- **模拟器检测**：检测使用模拟器注册

## 8. 性能优化

### 请求处理优化
- **异步处理**：验证码发送等操作异步处理
- **批量处理**：批量写入数据库
- **连接池**：使用连接池复用连接

### 缓存优化
- **验证码缓存**：验证码存储在Redis
- **热点数据缓存**：热门账号信息缓存
- **限流缓存**：限流计数器缓存

### 数据库优化
- **索引优化**：为常用查询创建索引
- **读写分离**：使用读写分离的数据库架构
- **分库分表**：用户量大的情况下分库分表

## 9. 错误码定义

```cpp
enum RegisterErrorCode {
    SUCCESS = 0,
    
    // 客户端错误
    ERROR_INVALID_USERNAME = 1001,    // 用户名格式错误
    ERROR_INVALID_PASSWORD = 1002,    // 密码格式错误
    ERROR_INVALID_PHONE = 1003,       // 手机号格式错误
    ERROR_INVALID_EMAIL = 1004,      // 邮箱格式错误
    ERROR_INVALID_CODE = 1005,        // 验证码错误
    
    // 业务错误
    ERROR_USERNAME_EXISTS = 2001,    // 用户名已存在
    ERROR_PHONE_EXISTS = 2002,       // 手机号已注册
    ERROR_EMAIL_EXISTS = 2003,       // 邮箱已注册
    ERROR_CODE_EXPIRED = 2004,       // 验证码已过期
    ERROR_CODE_USED = 2005,          // 验证码已使用
    ERROR_TOO_MANY_REQUESTS = 2006,  // 请求过于频繁
    ERROR_REGISTER_LIMIT = 2007,     // 注册次数超限
    
    // 服务端错误
    ERROR_SEND_FAILED = 3001,        // 发送失败
    ERROR_CREATE_USER = 3002,         // 创建用户失败
    ERROR_DATABASE = 3003,           // 数据库错误
    ERROR_INTERNAL = 3004,          // 内部错误
};
```

## 10. 配置参数

```yaml
server:
  host: "0.0.0.0"
  port: 8080
  worker_count: 4

redis:
  host: "127.0.0.1"
  port: 6379
  password: ""
  db: 0
  pool_size: 10

database:
  host: "localhost"
  port: 3306
  user: "root"
  password: "password"
  database: "game_db"
  pool_size: 10
  enable_read_write_split: true

sms:
  provider: "aliyun"
  app_key: "xxx"
  app_secret: "xxx"
  sign_name: "游戏名称"
  template_code: "SMS_xxx"

email:
  smtp_host: "smtp.example.com"
  smtp_port: 465
  username: "noreply@example.com"
  password: "xxx"
  from_name: "游戏名称"

register:
  username_min_length: 4
  username_max_length: 20
  password_min_length: 8
  password_max_length: 32
  code_length: 6
  code_expire_time: 300
  send_interval: 60
  max_send_per_day: 10
  max_register_per_ip_per_day: 10
  max_register_per_device_per_day: 2

reward:
  enable: true
  gold: 1000
  items: [1001, 1002]
  experience: 100
```

本设计方案提供了一个完整的C++ MMO游戏玩家服务器注册模块流程设计，涵盖了系统架构、完整流程、核心数据结构、实现代码、数据库设计、Redis缓存、安全措施、性能优化、错误码定义和配置参数等各个方面。
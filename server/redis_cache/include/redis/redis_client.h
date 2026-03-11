#pragma once

#include <string>
#include <vector>
#include <memory>

// 前向声明
typedef struct redisContext redisContext;
typedef struct redisReply redisReply;

namespace redis {

class RedisClient {
public:
    RedisClient();
    ~RedisClient();
    
    bool connect(const std::string& host, int port);
    void disconnect();
    
    bool set(const std::string& key, const std::string& value);
    bool setex(const std::string& key, int seconds, const std::string& value);
    std::string get(const std::string& key);
    bool del(const std::string& key);
    bool exists(const std::string& key);
    
    bool hset(const std::string& key, const std::string& field, const std::string& value);
    std::string hget(const std::string& key, const std::string& field);
    bool hdel(const std::string& key, const std::string& field);
    std::vector<std::string> hgetall(const std::string& key);
    
    bool expire(const std::string& key, int seconds);
    int ttl(const std::string& key);
    
    std::string getLastError() const;
    
private:
    redisContext* context_;
    std::string last_error_;
    
    void freeReply(redisReply* reply);
};

} // namespace redis

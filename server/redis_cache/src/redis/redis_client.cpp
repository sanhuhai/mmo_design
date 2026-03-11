#include "redis/redis_client.h"
#include <hiredis/hiredis.h>
#include <cstring>
#include <iostream>

namespace redis {

RedisClient::RedisClient() : context_(nullptr) {
}

RedisClient::~RedisClient() {
    disconnect();
}

bool RedisClient::connect(const std::string& host, int port) {
    disconnect();
    
    context_ = redisConnect(host.c_str(), port);
    if (context_ == nullptr || context_->err) {
        if (context_) {
            last_error_ = context_->errstr;
            redisFree(context_);
            context_ = nullptr;
        } else {
            last_error_ = "Failed to allocate redis context";
        }
        return false;
    }
    
    return true;
}

void RedisClient::disconnect() {
    if (context_) {
        redisFree(context_);
        context_ = nullptr;
    }
    last_error_.clear();
}

void RedisClient::freeReply(redisReply* reply) {
    if (reply) {
        freeReplyObject(reply);
    }
}

bool RedisClient::set(const std::string& key, const std::string& value) {
    if (!context_) {
        last_error_ = "Not connected";
        return false;
    }
    
    redisReply* reply = (redisReply*)redisCommand(context_, "SET %s %b", key.c_str(), value.c_str(), value.size());
    if (!reply) {
        last_error_ = context_->errstr;
        return false;
    }
    
    bool success = (reply->type == REDIS_REPLY_STATUS && strcmp(reply->str, "OK") == 0);
    if (!success) {
        last_error_ = reply->str;
    }
    
    freeReply(reply);
    return success;
}

bool RedisClient::setex(const std::string& key, int seconds, const std::string& value) {
    if (!context_) {
        last_error_ = "Not connected";
        return false;
    }
    
    redisReply* reply = (redisReply*)redisCommand(context_, "SETEX %s %d %b", key.c_str(), seconds, value.c_str(), value.size());
    if (!reply) {
        last_error_ = context_->errstr;
        return false;
    }
    
    bool success = (reply->type == REDIS_REPLY_STATUS && strcmp(reply->str, "OK") == 0);
    if (!success) {
        last_error_ = reply->str;
    }
    
    freeReply(reply);
    return success;
}

std::string RedisClient::get(const std::string& key) {
    if (!context_) {
        last_error_ = "Not connected";
        return "";
    }
    
    redisReply* reply = (redisReply*)redisCommand(context_, "GET %s", key.c_str());
    if (!reply) {
        last_error_ = context_->errstr;
        return "";
    }
    
    std::string result;
    if (reply->type == REDIS_REPLY_STRING) {
        result = reply->str;
    } else if (reply->type == REDIS_REPLY_NIL) {
        result = "";
    } else {
        last_error_ = "Unexpected reply type";
    }
    
    freeReply(reply);
    return result;
}

bool RedisClient::del(const std::string& key) {
    if (!context_) {
        last_error_ = "Not connected";
        return false;
    }
    
    redisReply* reply = (redisReply*)redisCommand(context_, "DEL %s", key.c_str());
    if (!reply) {
        last_error_ = context_->errstr;
        return false;
    }
    
    bool success = (reply->type == REDIS_REPLY_INTEGER);
    freeReply(reply);
    return success;
}

bool RedisClient::exists(const std::string& key) {
    if (!context_) {
        last_error_ = "Not connected";
        return false;
    }
    
    redisReply* reply = (redisReply*)redisCommand(context_, "EXISTS %s", key.c_str());
    if (!reply) {
        last_error_ = context_->errstr;
        return false;
    }
    
    bool exists = (reply->type == REDIS_REPLY_INTEGER && reply->integer > 0);
    freeReply(reply);
    return exists;
}

bool RedisClient::hset(const std::string& key, const std::string& field, const std::string& value) {
    if (!context_) {
        last_error_ = "Not connected";
        return false;
    }
    
    redisReply* reply = (redisReply*)redisCommand(context_, "HSET %s %s %b", key.c_str(), field.c_str(), value.c_str(), value.size());
    if (!reply) {
        last_error_ = context_->errstr;
        return false;
    }
    
    bool success = (reply->type == REDIS_REPLY_INTEGER);
    freeReply(reply);
    return success;
}

std::string RedisClient::hget(const std::string& key, const std::string& field) {
    if (!context_) {
        last_error_ = "Not connected";
        return "";
    }
    
    redisReply* reply = (redisReply*)redisCommand(context_, "HGET %s %s", key.c_str(), field.c_str());
    if (!reply) {
        last_error_ = context_->errstr;
        return "";
    }
    
    std::string result;
    if (reply->type == REDIS_REPLY_STRING) {
        result = reply->str;
    } else if (reply->type == REDIS_REPLY_NIL) {
        result = "";
    } else {
        last_error_ = "Unexpected reply type";
    }
    
    freeReply(reply);
    return result;
}

bool RedisClient::hdel(const std::string& key, const std::string& field) {
    if (!context_) {
        last_error_ = "Not connected";
        return false;
    }
    
    redisReply* reply = (redisReply*)redisCommand(context_, "HDEL %s %s", key.c_str(), field.c_str());
    if (!reply) {
        last_error_ = context_->errstr;
        return false;
    }
    
    bool success = (reply->type == REDIS_REPLY_INTEGER);
    freeReply(reply);
    return success;
}

std::vector<std::string> RedisClient::hgetall(const std::string& key) {
    std::vector<std::string> result;
    
    if (!context_) {
        last_error_ = "Not connected";
        return result;
    }
    
    redisReply* reply = (redisReply*)redisCommand(context_, "HGETALL %s", key.c_str());
    if (!reply) {
        last_error_ = context_->errstr;
        return result;
    }
    
    if (reply->type == REDIS_REPLY_ARRAY) {
        for (size_t i = 0; i < reply->elements; ++i) {
            if (reply->element[i]->type == REDIS_REPLY_STRING) {
                result.push_back(reply->element[i]->str);
            }
        }
    }
    
    freeReply(reply);
    return result;
}

bool RedisClient::expire(const std::string& key, int seconds) {
    if (!context_) {
        last_error_ = "Not connected";
        return false;
    }
    
    redisReply* reply = (redisReply*)redisCommand(context_, "EXPIRE %s %d", key.c_str(), seconds);
    if (!reply) {
        last_error_ = context_->errstr;
        return false;
    }
    
    bool success = (reply->type == REDIS_REPLY_INTEGER && reply->integer > 0);
    freeReply(reply);
    return success;
}

int RedisClient::ttl(const std::string& key) {
    if (!context_) {
        last_error_ = "Not connected";
        return -1;
    }
    
    redisReply* reply = (redisReply*)redisCommand(context_, "TTL %s", key.c_str());
    if (!reply) {
        last_error_ = context_->errstr;
        return -1;
    }
    
    int ttl = -1;
    if (reply->type == REDIS_REPLY_INTEGER) {
        ttl = reply->integer;
    }
    
    freeReply(reply);
    return ttl;
}

std::string RedisClient::getLastError() const {
    return last_error_;
}

} // namespace redis

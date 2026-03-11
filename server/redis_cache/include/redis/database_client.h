#pragma once

#include <string>
#include <vector>
#include <memory>

// 前向声明
typedef struct MYSQL MYSQL;

typedef struct MYSQL_RES MYSQL_RES;
typedef struct MYSQL_ROW MYSQL_ROW;

namespace redis {

class DatabaseClient {
public:
    DatabaseClient();
    ~DatabaseClient();
    
    bool connect(const std::string& host, int port,
                 const std::string& user, const std::string& password,
                 const std::string& db_name);
    void disconnect();
    
    bool execute(const std::string& sql);
    bool query(const std::string& sql, std::vector<std::vector<std::string>>& results);
    
    bool insertPlayerData(int player_id, const std::string& serialized_data);
    bool updatePlayerData(int player_id, const std::string& serialized_data);
    bool selectPlayerData(int player_id, std::string& serialized_data);
    bool deletePlayerData(int player_id);
    
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();
    
    std::string getLastError() const;
    
private:
    MYSQL* connection_;
    std::string last_error_;
    
    bool isConnected();
    void freeResult(MYSQL_RES* result);
};

} // namespace redis

#include "redis/database_client.h"
#include <mysql/mysql.h>
#include <cstring>
#include <iostream>
#include <chrono>

namespace redis {

DatabaseClient::DatabaseClient() : connection_(nullptr) {
}

DatabaseClient::~DatabaseClient() {
    disconnect();
}

bool DatabaseClient::connect(const std::string& host, int port,
                            const std::string& user, const std::string& password,
                            const std::string& db_name) {
    disconnect();
    
    connection_ = mysql_init(nullptr);
    if (!connection_) {
        last_error_ = "Failed to initialize MySQL connection";
        return false;
    }
    
    if (!mysql_real_connect(connection_, host.c_str(), user.c_str(), password.c_str(),
                           db_name.c_str(), port, nullptr, 0)) {
        last_error_ = mysql_error(connection_);
        mysql_close(connection_);
        connection_ = nullptr;
        return false;
    }
    
    // 设置字符集
    if (mysql_set_character_set(connection_, "utf8mb4")) {
        last_error_ = mysql_error(connection_);
    }
    
    return true;
}

void DatabaseClient::disconnect() {
    if (connection_) {
        mysql_close(connection_);
        connection_ = nullptr;
    }
    last_error_.clear();
}

bool DatabaseClient::isConnected() {
    return connection_ != nullptr && mysql_ping(connection_) == 0;
}

void DatabaseClient::freeResult(MYSQL_RES* result) {
    if (result) {
        mysql_free_result(result);
    }
}

bool DatabaseClient::execute(const std::string& sql) {
    if (!isConnected()) {
        last_error_ = "Not connected";
        return false;
    }
    
    if (mysql_query(connection_, sql.c_str())) {
        last_error_ = mysql_error(connection_);
        return false;
    }
    
    return true;
}

bool DatabaseClient::query(const std::string& sql, std::vector<std::vector<std::string>>& results) {
    if (!isConnected()) {
        last_error_ = "Not connected";
        return false;
    }
    
    if (mysql_query(connection_, sql.c_str())) {
        last_error_ = mysql_error(connection_);
        return false;
    }
    
    MYSQL_RES* result = mysql_store_result(connection_);
    if (!result) {
        last_error_ = mysql_error(connection_);
        return false;
    }
    
    int num_fields = mysql_num_fields(result);
    MYSQL_ROW row;
    
    while ((row = mysql_fetch_row(result))) {
        std::vector<std::string> row_data;
        for (int i = 0; i < num_fields; ++i) {
            if (row[i]) {
                row_data.push_back(row[i]);
            } else {
                row_data.push_back("");
            }
        }
        results.push_back(row_data);
    }
    
    freeResult(result);
    return true;
}

bool DatabaseClient::insertPlayerData(int player_id, const std::string& serialized_data) {
    std::string sql = "INSERT INTO player_data (player_id, data, update_time, create_time) VALUES (";
    sql += std::to_string(player_id) + ", '" + serialized_data + "', ";
    sql += std::to_string(std::chrono::system_clock::now().time_since_epoch().count() / 1000) + ", ";
    sql += std::to_string(std::chrono::system_clock::now().time_since_epoch().count() / 1000) + ")";
    
    return execute(sql);
}

bool DatabaseClient::updatePlayerData(int player_id, const std::string& serialized_data) {
    std::string sql = "UPDATE player_data SET data = '" + serialized_data + "', ";
    sql += "update_time = " + std::to_string(std::chrono::system_clock::now().time_since_epoch().count() / 1000);
    sql += " WHERE player_id = " + std::to_string(player_id);
    
    return execute(sql);
}

bool DatabaseClient::selectPlayerData(int player_id, std::string& serialized_data) {
    std::string sql = "SELECT data FROM player_data WHERE player_id = " + std::to_string(player_id);
    std::vector<std::vector<std::string>> results;
    
    if (!query(sql, results)) {
        return false;
    }
    
    if (results.empty() || results[0].empty()) {
        last_error_ = "Player data not found";
        return false;
    }
    
    serialized_data = results[0][0];
    return true;
}

bool DatabaseClient::deletePlayerData(int player_id) {
    std::string sql = "DELETE FROM player_data WHERE player_id = " + std::to_string(player_id);
    return execute(sql);
}

bool DatabaseClient::beginTransaction() {
    return execute("START TRANSACTION");
}

bool DatabaseClient::commitTransaction() {
    return execute("COMMIT");
}

bool DatabaseClient::rollbackTransaction() {
    return execute("ROLLBACK");
}

std::string DatabaseClient::getLastError() const {
    return last_error_;
}

} // namespace redis

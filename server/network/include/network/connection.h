#ifndef CONNECTION_H
#define CONNECTION_H

#include <uv.h>
#include <string>
#include <chrono>

namespace network {

class Connection {
public:
    Connection(uv_tcp_t* client);
    ~Connection();

    std::string getClientAddress() const;
    bool checkHeartbeat();
    void updateHeartbeat();
    uv_tcp_t* getClient() const;

private:
    uv_tcp_t* client_;
    std::string client_address_;
    std::chrono::steady_clock::time_point last_heartbeat_;
};

} // namespace network

#endif // CONNECTION_H
#include "network/connection.h"
#include <cstring>
#include <sstream>

namespace network {

Connection::Connection(uv_tcp_t* client) : client_(client) {
    sockaddr_storage addr;
    int len = sizeof(addr);
    uv_tcp_getpeername(client, (sockaddr*)&addr, &len);
    
    char ip[16];
    int port;
    if (addr.ss_family == AF_INET) {
        sockaddr_in* ipv4 = (sockaddr_in*)&addr;
        uv_ip4_name(ipv4, ip, sizeof(ip));
        port = ntohs(ipv4->sin_port);
    } else {
        sockaddr_in6* ipv6 = (sockaddr_in6*)&addr;
        uv_ip6_name(ipv6, ip, sizeof(ip));
        port = ntohs(ipv6->sin6_port);
    }
    
    std::stringstream ss;
    ss << ip << ":" << port;
    client_address_ = ss.str();
    
    updateHeartbeat();
}

Connection::~Connection() {
}

std::string Connection::getClientAddress() const {
    return client_address_;
}

bool Connection::checkHeartbeat() {
    auto now = std::chrono::steady_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::seconds>(now - last_heartbeat_);
    return diff.count() <= 30;
}

void Connection::updateHeartbeat() {
    last_heartbeat_ = std::chrono::steady_clock::now();
}

uv_tcp_t* Connection::getClient() const {
    return client_;
}

} // namespace network
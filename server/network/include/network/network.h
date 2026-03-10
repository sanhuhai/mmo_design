#ifndef NETWORK_H
#define NETWORK_H

namespace network {

enum class ErrorCode {
    OK = 0,
    INVALID_PARAM = 1,
    NETWORK_ERROR = 2,
    INTERNAL_ERROR = 3
};

// 初始化网络库
bool init();

// 清理网络库
void cleanup();

} // namespace network

#endif // NETWORK_H
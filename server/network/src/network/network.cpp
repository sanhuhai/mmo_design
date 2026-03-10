#include "network/network.h"
#include <uv.h>

namespace network {

bool init() {
    // 初始化LibUV
    return true;
}

void cleanup() {
    // 清理LibUV资源
}

} // namespace network
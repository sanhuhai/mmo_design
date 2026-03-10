cmake_minimum_required(VERSION 3.10)
project(network_module)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置第三方库路径
set(THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party)

# 查找LibUV
find_path(LIBUV_INCLUDE_DIR uv.h PATHS ${THIRD_PARTY_DIR}/libuv/include)
find_library(LIBUV_LIBRARY uv PATHS ${THIRD_PARTY_DIR}/libuv/build/Release)

if(NOT LIBUV_INCLUDE_DIR OR NOT LIBUV_LIBRARY)
    message(FATAL_ERROR "LibUV not found. Please download and build LibUV first.")
endif()

# 包含头文件目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${LIBUV_INCLUDE_DIR}
)

# 源文件
set(SOURCES
    src/test.cpp
    src/network/network.cpp
    src/network/connection.cpp
    src/network/message.cpp
    src/network/server.cpp
)

# 构建可执行文件
add_executable(server_example ${SOURCES})

# 链接库
target_link_libraries(server_example ${LIBUV_LIBRARY} ws2_32)

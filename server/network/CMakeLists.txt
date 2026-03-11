cmake_minimum_required(VERSION 3.10)
project(network_module)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置第三方库路径
set(THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../third_party)

# 查找LibUV - 尝试多个可能的路径
find_path(LIBUV_INCLUDE_DIR uv.h 
    PATHS 
        ${THIRD_PARTY_DIR}/libuv/include
        ${THIRD_PARTY_DIR}/libuv/build/include
        /usr/include
        /usr/local/include
)

find_library(LIBUV_LIBRARY uv 
    PATHS 
        ${THIRD_PARTY_DIR}/libuv/build/Release
        ${THIRD_PARTY_DIR}/libuv/build
        ${THIRD_PARTY_DIR}/libuv/build/lib
        /usr/lib
        /usr/local/lib
        /usr/lib/x86_64-linux-gnu
)

if(NOT LIBUV_INCLUDE_DIR OR NOT LIBUV_LIBRARY)
    # 尝试从父目录查找
    find_path(LIBUV_INCLUDE_DIR uv.h 
        PATHS 
            ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libuv/include
            ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libuv/build/include
    )
    find_library(LIBUV_LIBRARY uv 
        PATHS 
            ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libuv/build/Release
            ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libuv/build
            ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libuv/build/lib
    )
endif()

if(NOT LIBUV_INCLUDE_DIR OR NOT LIBUV_LIBRARY)
    message(WARNING "LibUV not found. Network module will be disabled.")
    set(BUILD_NETWORK_MODULE OFF PARENT_SCOPE)
    return()
endif()

message(STATUS "LibUV found:")
message(STATUS "  Include: ${LIBUV_INCLUDE_DIR}")
message(STATUS "  Library: ${LIBUV_LIBRARY}")

# 包含头文件目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${LIBUV_INCLUDE_DIR}
)

# 源文件
set(SOURCES
    src/network/network.cpp
    src/network/connection.cpp
    src/network/message.cpp
    src/network/server.cpp
)

# 构建静态库
add_library(network_module STATIC ${SOURCES})

# 根据平台链接不同的库
if(WIN32)
    target_link_libraries(network_module ${LIBUV_LIBRARY} ws2_32)
else()
    target_link_libraries(network_module ${LIBUV_LIBRARY} pthread)
endif()

# 构建示例程序
add_executable(server_example examples/server_example.cpp)
if(WIN32)
    target_link_libraries(server_example network_module ${LIBUV_LIBRARY} ws2_32)
else()
    target_link_libraries(server_example network_module ${LIBUV_LIBRARY} pthread)
endif()

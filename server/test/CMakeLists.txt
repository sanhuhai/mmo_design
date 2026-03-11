cmake_minimum_required(VERSION 3.10)
project(network_test)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 Protobuf 库
find_package(Protobuf REQUIRED)
if(PROTOBUF_FOUND)
    message(STATUS "Protobuf found: ${PROTOBUF_VERSION}")
    message(STATUS "Protobuf include: ${PROTOBUF_INCLUDE_DIRS}")
    message(STATUS "Protobuf libraries: ${PROTOBUF_LIBRARIES}")
    message(STATUS "Protobuf protoc: ${PROTOBUF_PROTOC_EXECUTABLE}")
    # 生成 Protobuf 文件
    set(PROTO_FILES proto/test.proto)
    protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})
    include_directories(${CMAKE_CURRENT_BINARY_DIR})
else()
    message(FATAL_ERROR "Protobuf not found, required for build")
endif()

# 包含网络模块
include_directories(
    ../network/include
    ${PROTOBUF_INCLUDE_DIRS}
)

# 链接网络模块
link_directories(
    ../build/network
)

# 构建服务器测试
add_executable(network_test_server network_test_server.cpp ${PROTO_SRCS})
target_link_libraries(network_test_server
    network_module
    ${PROTOBUF_LIBRARIES}
    uv
)

# 构建客户端测试
add_executable(network_test_client network_test_client.cpp ${PROTO_SRCS})
target_link_libraries(network_test_client
    network_module
    ${PROTOBUF_LIBRARIES}
    uv
)

# 宏定义
target_compile_definitions(network_test_server PRIVATE PROTOBUF_FOUND)
target_compile_definitions(network_test_client PRIVATE PROTOBUF_FOUND)

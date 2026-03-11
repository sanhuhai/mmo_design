cmake_minimum_required(VERSION 3.10)
project(game_server)

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
    set(PROTO_FILES proto/game.proto)
    protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})
    include_directories(${CMAKE_CURRENT_BINARY_DIR})
else()
    message(FATAL_ERROR "Protobuf not found, required for build")
endif()

# 包含目录
include_directories(
    include
    ${PROTOBUF_INCLUDE_DIRS}
    ../network/include
)

# 链接目录
link_directories(
    ../build/network
)

# 源文件
set(SOURCE_FILES
    src/game/user_manager.cpp
    src/game/game_server.cpp
    ${PROTO_SRCS}
)

# 头文件
set(HEADER_FILES
    include/game/user_manager.h
    include/game/game_server.h
    ${PROTO_HDRS}
)

# 构建静态库
add_library(game_server STATIC ${SOURCE_FILES} ${HEADER_FILES})

# 链接库
target_link_libraries(game_server
    ${PROTOBUF_LIBRARIES}
    network_module
    uv
)

# 构建示例程序
add_executable(game_server_example examples/game_server_example.cpp)
target_link_libraries(game_server_example game_server)

# 宏定义
target_compile_definitions(game_server PRIVATE PROTOBUF_FOUND)

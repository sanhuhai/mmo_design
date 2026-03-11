cmake_minimum_required(VERSION 3.10)
project(chat_service)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 Protobuf 库
find_package(Protobuf REQUIRED)
if(PROTOBUF_FOUND)
    message(STATUS "Protobuf found: ${PROTOBUF_VERSION}")
    message(STATUS "Protobuf include: ${PROTOBUF_INCLUDE_DIRS}")
    message(STATUS "Protobuf libraries: ${PROTOBUF_LIBRARIES}")
    message(STATUS "Protobuf protoc: ${PROTOBUF_PROTOC_EXECUTABLE}")
else()
    message(WARNING "Protobuf not found, using fallback implementation")
endif()

# 查找 Redis 库
find_path(HIREDIS_INCLUDE_DIR hiredis/hiredis.h)
find_library(HIREDIS_LIBRARY hiredis)
if(HIREDIS_INCLUDE_DIR AND HIREDIS_LIBRARY)
    message(STATUS "hiredis found:")
    message(STATUS "  Include: ${HIREDIS_INCLUDE_DIR}")
    message(STATUS "  Library: ${HIREDIS_LIBRARY}")
    set(ENABLE_REDIS ON)
else()
    message(WARNING "hiredis not found, Redis support disabled")
    set(ENABLE_REDIS OFF)
endif()

# 查找 MySQL 库
find_path(MYSQL_INCLUDE_DIR mysql/mysql.h)
find_library(MYSQL_LIBRARY mysqlclient)
if(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARY)
    message(STATUS "MySQL found:")
    message(STATUS "  Include: ${MYSQL_INCLUDE_DIR}")
    message(STATUS "  Library: ${MYSQL_LIBRARY}")
    set(ENABLE_DATABASE ON)
else()
    message(WARNING "MySQL not found, database support disabled")
    set(ENABLE_DATABASE OFF)
endif()

# 生成 Protobuf 文件
if(PROTOBUF_FOUND)
    set(PROTO_FILES proto/chat.proto)
    protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})
    include_directories(${CMAKE_CURRENT_BINARY_DIR})
endif()

# 包含目录
include_directories(
    include
    ${PROTOBUF_INCLUDE_DIRS}
    ${HIREDIS_INCLUDE_DIR}
    ${MYSQL_INCLUDE_DIR}
    ../redis_cache/include
)

# 源文件
set(SOURCE_FILES
    src/chat/chat_server.cpp
    src/chat/channel.cpp
    src/chat/message_router.cpp
    src/chat/content_filter.cpp
    src/chat/client_session.cpp
    ${PROTO_SRCS}
)

# 头文件
set(HEADER_FILES
    include/chat/chat_server.h
    include/chat/channel.h
    include/chat/message_router.h
    include/chat/content_filter.h
    include/chat/client_session.h
    include/chat/chat_message.h
    ${PROTO_HDRS}
)

# 构建静态库
add_library(chat_service STATIC ${SOURCE_FILES} ${HEADER_FILES})

# 链接库
target_link_libraries(chat_service
    ${PROTOBUF_LIBRARIES}
    ${HIREDIS_LIBRARY}
    ${MYSQL_LIBRARY}
    redis_cache
)

# 构建示例程序
add_executable(chat_example examples/chat_example.cpp)
target_link_libraries(chat_example chat_service)

# 宏定义
if(PROTOBUF_FOUND)
    target_compile_definitions(chat_service PRIVATE PROTOBUF_FOUND)
endif()

if(ENABLE_REDIS)
    target_compile_definitions(chat_service PRIVATE ENABLE_REDIS)
endif()

if(ENABLE_DATABASE)
    target_compile_definitions(chat_service PRIVATE ENABLE_DATABASE)
endif()

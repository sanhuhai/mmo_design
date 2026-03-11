cmake_minimum_required(VERSION 3.10)
project(redis_cache)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 包含Protobuf的CMake模块
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/protobuf/cmake")

# 查找Protobuf
find_package(Protobuf 3.0.0)

# 手动设置protoc可执行文件路径
if(Protobuf_FOUND AND NOT Protobuf_PROTOC_EXECUTABLE)
    find_program(Protobuf_PROTOC_EXECUTABLE protoc
        PATHS
            ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/protobuf/build/Release
            ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/protobuf/build/bin
    )
endif()

if(Protobuf_FOUND AND Protobuf_PROTOC_EXECUTABLE)
    message(STATUS "Protobuf found: ${Protobuf_VERSION}")
    message(STATUS "Protobuf include: ${Protobuf_INCLUDE_DIRS}")
    message(STATUS "Protobuf libraries: ${Protobuf_LIBRARIES}")
    message(STATUS "Protobuf protoc: ${Protobuf_PROTOC_EXECUTABLE}")
    set(ENABLE_PROTOBUF ON)
else()
    # 尝试手动查找本地编译的版本
    find_path(Protobuf_INCLUDE_DIR google/protobuf/message.h
        PATHS
            ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/protobuf/src
            ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/protobuf/build/src
    )

    find_library(Protobuf_LIBRARY protobuf
        PATHS
            ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/protobuf/build/Release
            ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/protobuf/build/lib
    )

    find_program(Protobuf_PROTOC_EXECUTABLE protoc
        PATHS
            ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/protobuf/build/Release
            ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/protobuf/build/bin
    )

    if(Protobuf_INCLUDE_DIR AND Protobuf_LIBRARY AND Protobuf_PROTOC_EXECUTABLE)
        set(Protobuf_FOUND ON)
        set(Protobuf_INCLUDE_DIRS ${Protobuf_INCLUDE_DIR})
        set(Protobuf_LIBRARIES ${Protobuf_LIBRARY})
        message(STATUS "Protobuf found (local build):")
        message(STATUS "  Include: ${Protobuf_INCLUDE_DIR}")
        message(STATUS "  Library: ${Protobuf_LIBRARY}")
        message(STATUS "  Protoc: ${Protobuf_PROTOC_EXECUTABLE}")
        set(ENABLE_PROTOBUF ON)
        
        # 手动生成proto文件
        set(PROTO_FILE ${CMAKE_CURRENT_SOURCE_DIR}/protos/player_data.proto)
        set(PROTO_HDR ${CMAKE_CURRENT_BINARY_DIR}/player_data.pb.h)
        set(PROTO_SRC ${CMAKE_CURRENT_BINARY_DIR}/player_data.pb.cc)
        
        add_custom_command(
            OUTPUT ${PROTO_HDR} ${PROTO_SRC}
            COMMAND ${Protobuf_PROTOC_EXECUTABLE}
            ARGS --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
                 --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/protos
                 ${PROTO_FILE}
            DEPENDS ${PROTO_FILE}
            COMMENT "Generating protobuf files"
        )
        
        set(PROTO_SRCS ${PROTO_SRC})
        set(PROTO_HDRS ${PROTO_HDR})
    else()
        message(WARNING "Protobuf not found, Redis cache system will be disabled")
        set(ENABLE_PROTOBUF OFF)
        set(BUILD_REDIS_CACHE OFF PARENT_SCOPE)
        return()
    endif()
endif()

# 直接使用add_custom_command生成proto文件
set(PROTO_FILE ${CMAKE_CURRENT_SOURCE_DIR}/protos/player_data.proto)
set(PROTO_HDR ${CMAKE_CURRENT_BINARY_DIR}/player_data.pb.h)
set(PROTO_SRC ${CMAKE_CURRENT_BINARY_DIR}/player_data.pb.cc)

add_custom_command(
    OUTPUT ${PROTO_HDR} ${PROTO_SRC}
    COMMAND ${Protobuf_PROTOC_EXECUTABLE}
    ARGS --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
         --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/protos
         ${PROTO_FILE}
    DEPENDS ${PROTO_FILE}
    COMMENT "Generating protobuf files"
)

set(PROTO_SRCS ${PROTO_SRC})
set(PROTO_HDRS ${PROTO_HDR})

# 查找hiredis (Redis客户端库)
find_path(HIREDIS_INCLUDE_DIR hiredis/hiredis.h
    PATHS
        ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/hiredis
        ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/hiredis/include
)

find_library(HIREDIS_LIBRARY hiredis
    PATHS
        ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/hiredis/build
        ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/hiredis
)

if(HIREDIS_INCLUDE_DIR AND HIREDIS_LIBRARY)
    message(STATUS "hiredis found:")
    message(STATUS "  Include: ${HIREDIS_INCLUDE_DIR}")
    message(STATUS "  Library: ${HIREDIS_LIBRARY}")
    set(ENABLE_REDIS ON)
else()
    message(WARNING "hiredis not found, Redis support will be disabled")
    set(ENABLE_REDIS OFF)
endif()

# 查找MySQL客户端库
find_path(MYSQL_INCLUDE_DIR mysql/mysql.h)
find_library(MYSQL_LIBRARY mysqlclient)

if(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARY)
    message(STATUS "MySQL found:")
    message(STATUS "  Include: ${MYSQL_INCLUDE_DIR}")
    message(STATUS "  Library: ${MYSQL_LIBRARY}")
else()
    message(WARNING "MySQL not found, database support will be disabled")
    set(ENABLE_DATABASE OFF)
endif()

# 包含头文件目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${Protobuf_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/abseil-cpp
)

if(ENABLE_REDIS)
    include_directories(${HIREDIS_INCLUDE_DIR})
endif()

if(ENABLE_DATABASE)
    include_directories(${MYSQL_INCLUDE_DIR})
endif()

# 源文件
set(SOURCES
    src/redis/protobuf_serializer.cpp
    src/redis/cache_manager.cpp
    ${PROTO_SRCS}
)

if(ENABLE_REDIS)
    list(APPEND SOURCES src/redis/redis_client.cpp)
endif()

if(ENABLE_DATABASE)
    list(APPEND SOURCES src/redis/database_client.cpp)
endif()

# 构建静态库
add_library(redis_cache STATIC ${SOURCES})

# 链接库
target_link_libraries(redis_cache ${Protobuf_LIBRARIES})

if(ENABLE_REDIS)
    target_link_libraries(redis_cache ${HIREDIS_LIBRARY})
endif()

if(ENABLE_DATABASE)
    target_link_libraries(redis_cache ${MYSQL_LIBRARY})
endif()

if(WIN32)
    target_link_libraries(redis_cache ws2_32)
else()
    target_link_libraries(redis_cache pthread)
endif()

# 构建示例程序
add_executable(cache_example examples/cache_example.cpp)
target_link_libraries(cache_example redis_cache)

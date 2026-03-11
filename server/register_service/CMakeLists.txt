cmake_minimum_required(VERSION 3.10)
project(register_service)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 包含Protobuf的CMake模块
find_package(Protobuf REQUIRED)
if(PROTOBUF_FOUND)
    message(STATUS "Protobuf found: ${PROTOBUF_VERSION}")
    message(STATUS "Protobuf include: ${PROTOBUF_INCLUDE_DIRS}")
    message(STATUS "Protobuf libraries: ${PROTOBUF_LIBRARIES}")
    message(STATUS "Protobuf protoc: ${PROTOBUF_PROTOC_EXECUTABLE}")
    # 生成protobuf文件
    protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS proto/register.proto)
else()
    message(FATAL_ERROR "Protobuf not found, required for build")
endif()

# 包含头文件目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../redis_cache/include
)

if(PROTOBUF_FOUND)
    include_directories(${PROTOBUF_INCLUDE_DIRS})
endif()

# 查找Redis客户端库
find_path(HIREDIS_INCLUDE_DIR hiredis/hiredis.h)
find_library(HIREDIS_LIBRARY hiredis)

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
    set(ENABLE_DATABASE ON)
else()
    message(WARNING "MySQL not found, database support will be disabled")
    set(ENABLE_DATABASE OFF)
endif()

# 源文件
set(SOURCE_FILES
    src/register/register_service.cpp
    src/register/validation_service.cpp
    src/register/verification_code_service.cpp
    src/register/message_handler.cpp
    ${PROTO_SRCS}
)

# 头文件
set(HEADER_FILES
    include/register/register_service.h
    include/register/validation_service.h
    include/register/verification_code_service.h
    include/register/message_handler.h
    ${PROTO_HDRS}
)

# 创建静态库
add_library(register_service STATIC ${SOURCE_FILES} ${HEADER_FILES})

# 链接依赖
target_link_libraries(register_service
    redis_cache
)

if(PROTOBUF_FOUND)
    target_link_libraries(register_service ${PROTOBUF_LIBRARIES})
    target_compile_definitions(register_service PRIVATE PROTOBUF_FOUND)
endif()

if(ENABLE_REDIS)
    target_link_libraries(register_service ${HIREDIS_LIBRARY})
    target_compile_definitions(register_service PRIVATE ENABLE_REDIS)
endif()

if(ENABLE_DATABASE)
    target_link_libraries(register_service ${MYSQL_LIBRARY})
    target_compile_definitions(register_service PRIVATE ENABLE_DATABASE)
endif()

# 示例程序
add_executable(register_example examples/register_example.cpp)
target_link_libraries(register_example register_service)

# 安装目标
install(TARGETS register_service
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES ${HEADER_FILES}
    DESTINATION include/register
)
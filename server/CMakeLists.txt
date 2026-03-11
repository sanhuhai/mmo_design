cmake_minimum_required(VERSION 3.10)
project(game_server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(CMAKE_BUILD_TYPE Debug)

option(BUILD_NETWORK_MODULE "Build network module" ON)
option(BUILD_CHARACTER_SYSTEM "Build character system" ON)
option(BUILD_REDIS_CACHE "Build Redis cache system" ON)
option(BUILD_REGISTER_SERVICE "Build register service" ON)
option(BUILD_CHAT_SERVICE "Build chat service" ON)
option(BUILD_GAME_SERVER "Build game server" ON)

if(BUILD_NETWORK_MODULE)
    add_subdirectory(network)
endif()

if(BUILD_CHARACTER_SYSTEM)
    add_subdirectory(character_system)
endif()

if(BUILD_REDIS_CACHE)
    add_subdirectory(redis_cache)
endif()

if(BUILD_REGISTER_SERVICE)
    add_subdirectory(register_service)
endif()

if(BUILD_CHAT_SERVICE)
    add_subdirectory(chat_service)
endif()

if(BUILD_GAME_SERVER)
    add_subdirectory(game)
endif()

# 构建测试
add_subdirectory(test)

option(BUILD_TESTS "Build tests" OFF)
if(BUILD_TESTS)
    enable_testing()
endif()

message(STATUS "Game Server Build Configuration:")
message(STATUS "  Build Network Module: ${BUILD_NETWORK_MODULE}")
message(STATUS "  Build Character System: ${BUILD_CHARACTER_SYSTEM}")
message(STATUS "  Build Redis Cache: ${BUILD_REDIS_CACHE}")
message(STATUS "  Build Register Service: ${BUILD_REGISTER_SERVICE}")
message(STATUS "  Build Chat Service: ${BUILD_CHAT_SERVICE}")
message(STATUS "  Build Game Server: ${BUILD_GAME_SERVER}")
message(STATUS "  Build Tests: ${BUILD_TESTS}")

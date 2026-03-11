cmake_minimum_required(VERSION 3.10)
project(game_server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_NETWORK_MODULE "Build network module" ON)
option(BUILD_CHARACTER_SYSTEM "Build character system" ON)
option(BUILD_REDIS_CACHE "Build Redis cache system" ON)

if(BUILD_NETWORK_MODULE)
    add_subdirectory(network)
endif()

if(BUILD_CHARACTER_SYSTEM)
    add_subdirectory(character_system)
endif()

if(BUILD_REDIS_CACHE)
    add_subdirectory(redis_cache)
endif()

option(BUILD_TESTS "Build tests" OFF)
if(BUILD_TESTS)
    enable_testing()
endif()

message(STATUS "Game Server Build Configuration:")
message(STATUS "  Build Network Module: ${BUILD_NETWORK_MODULE}")
message(STATUS "  Build Character System: ${BUILD_CHARACTER_SYSTEM}")
message(STATUS "  Build Redis Cache: ${BUILD_REDIS_CACHE}")
message(STATUS "  Build Tests: ${BUILD_TESTS}")

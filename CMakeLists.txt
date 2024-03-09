cmake_minimum_required(VERSION 3.13)  
project(shm_hash_map)              
set(CMAKE_CXX_STANDARD 17)           
set(CMAKE_XCODE_SCHEME_THREAD_SANITIZER ON)


include_directories(src)

set(SERVER_SOURCE_FILES src/server/main.cpp src/server/HashMap.cpp src/server/Server.cpp src/server/ThreadPool.cpp)

set(CLIENT_SOURCE_FILES  src/client/Client.cpp src/client/CyclicBufferShm.cpp)

add_executable(server ${SERVER_SOURCE_FILES})

add_executable(client ${CLIENT_SOURCE_FILES} src/client/main.cpp)

add_executable(perf_test_client ${CLIENT_SOURCE_FILES} src/client/perfTest.cpp)

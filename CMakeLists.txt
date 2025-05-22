cmake_minimum_required(VERSION 3.10)
project(RiddleGame)

set(CMAKE_CXX_STANDARD 17)
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

add_executable(server
    ${SRC_DIR}/main_server.cpp
    ${SRC_DIR}/server.cpp
    ${SRC_DIR}/protocol.cpp
)
target_include_directories(server PRIVATE ${SRC_DIR})

add_executable(client
    ${SRC_DIR}/main_client.cpp
    ${SRC_DIR}/client.cpp
    ${SRC_DIR}/protocol.cpp
)
target_include_directories(client PRIVATE ${SRC_DIR})

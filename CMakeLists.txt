cmake_minimum_required(VERSION 4.2)
project(CurriculumDesign)

set(CMAKE_CXX_STANDARD 20)

add_executable(Server)
target_sources(Server
    PRIVATE
        logger/Logger.cpp
        server/Server.cpp
    PUBLIC
        server/main.cpp
)

add_executable(Client)
target_sources(Client
    PRIVATE
        logger/Logger.cpp
        client/Client.cpp
    PUBLIC
        client/main.cpp
)
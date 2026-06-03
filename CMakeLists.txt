cmake_minimum_required(VERSION 4.2)
project(CurriculumDesign)

set(CMAKE_CXX_STANDARD 20)

add_executable(Server)
target_sources(Server
    PRIVATE
        logger/Logger.cpp
        stream/Stream.cpp
        stream/Parser.cpp
    PUBLIC
        server/Server.cpp
        server/main.cpp
)

add_executable(Client)
target_sources(Client
    PRIVATE
        logger/Logger.cpp
        stream/Stream.cpp
        stream/Parser.cpp
    PUBLIC
        client/Client.cpp
        client/main.cpp
)
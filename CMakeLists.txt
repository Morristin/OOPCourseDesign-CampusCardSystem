cmake_minimum_required(VERSION 4.2)
project(CurriculumDesign)

set(CMAKE_CXX_STANDARD 20)

add_executable(Server)
find_package(SQLite3 REQUIRED)
target_link_libraries(Server PRIVATE SQLite::SQLite3)
target_sources(Server
    PRIVATE
        logger/Logger.cpp
        stream/Stream.cpp
        stream/Parser.cpp
        stream/protocol.h
    PUBLIC
        server/Database.cpp
        server/Server.cpp
        server/main.cpp
)

add_executable(Client)
target_sources(Client
    PRIVATE
        logger/Logger.cpp
        stream/Stream.cpp
        stream/Parser.cpp
        stream/protocol.h
    PUBLIC
        client/Client.cpp
        client/main.cpp
)
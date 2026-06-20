cmake_minimum_required(VERSION 4.2)
project(CurriculumDesign)

set(CMAKE_CXX_STANDARD 20)

add_executable(Server)
target_compile_definitions(Server PRIVATE LOG_FILE="server.log")
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
target_compile_definitions(Client PRIVATE LOG_FILE="client.log")

target_sources(Client
        PRIVATE
        logger/Logger.cpp
        stream/Stream.cpp
        stream/Parser.cpp
        stream/protocol.h
        client/Users.cpp
        PUBLIC
        client/Client.cpp
        client/main.cpp
)
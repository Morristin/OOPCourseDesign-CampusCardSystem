# CMake Version Requirement & Project Settings
cmake_minimum_required(VERSION 4.2)
project(CurriculumDesign)

set(CMAKE_CXX_STANDARD 20)


# Server compile options & SQLite3 library auto detect.
add_executable(Server)
target_compile_definitions(Server PRIVATE LOG_FILE="server.log")

message(CHECK_START "Looking for local SQLite3 library.")
find_package(SQLite3 QUIET)
if (SQLite3_FOUND)
    message(CHECK_PASS "Successfully detect local SQLite3 library.")
    target_link_libraries(Server PRIVATE SQLite::SQLite3)
else ()
    message(CHECK_FAIL "Failed to find local SQLite3 library. Use amalgamation version in the repo instead.")
    target_sources(Server PRIVATE .third_party/sqlite3/sqlite3.c)
endif ()

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


# Client compile options
add_executable(Client)
target_compile_definitions(Client PRIVATE LOG_FILE="client.log")

target_sources(Client
        PRIVATE
        logger/Logger.cpp
        stream/Stream.cpp
        stream/Parser.cpp
        stream/protocol.h
        client/Users.cpp
        client/SuperOperator.cpp
        PUBLIC
        client/Client.cpp
        client/main.cpp
)
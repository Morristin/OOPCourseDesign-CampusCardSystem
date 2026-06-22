cmake_minimum_required(VERSION 4.2)
project(CurriculumDesign)

set(CMAKE_CXX_STANDARD 20)


add_executable(Server)
add_executable(Client)

target_compile_definitions(Server PRIVATE DEBUG_MODE=0)
target_compile_definitions(Client PRIVATE DEBUG_MODE=0)

target_compile_definitions(Server PRIVATE LOG_FILE="server.log")
target_compile_definitions(Client PRIVATE LOG_FILE="client.log")


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
        protocol/protocol.h
        protocol/colors.h
        PUBLIC
        server/Database.cpp
        server/Server.cpp
        server/main.cpp
)

target_sources(Client
        PRIVATE
        logger/Logger.cpp
        stream/Stream.cpp
        stream/Parser.cpp
        protocol/protocol.h
        protocol/colors.h
        client/Users.cpp
        client/SuperOperator.cpp
        client/Operator.cpp
        client/Student.cpp
        PUBLIC
        client/Client.cpp
        client/main.cpp
)
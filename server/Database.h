#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include <string>

#include "../stream/protocol.h"

struct DatabaseException : std::exception {
private:
    std::string_view message;

public:
    explicit DatabaseException(const std::string_view message) : message(message) { }
    [[nodiscard]] const char* what() const noexcept override { return message.data(); }
};

class Database {
private:
    static constexpr std::string DATABASE_PATH = "server/data.sqlite3";
    sqlite3* database { };

    static std::mutex database_mutex;
    sqlite3_stmt* cursor { };

public:
    explicit Database(const std::string& database_path);
    Database() : Database(DATABASE_PATH) { };
    ~Database() { sqlite3_close(database); }

    void initialize() const;
    LoginUserStatus check_identity(const std::string& username, const std::string& password);

    void add_operator(const std::string& username, const std::string& password);
    void del_operator(const std::string& username);
};

#endif

#include "Database.h"

#include "../logger/Logger.h"
#include "../stream/protocol.h"

static auto logger = Logger(__FILE__);

Database::Database(const std::string& database_path)
{
    if (const int status = sqlite3_open(database_path.c_str(), &database); status != SQLITE_OK) {
        logger.critical(std::format("Can not connect to database: {}", database_path));
    }
}

void Database::initialize() const
{
    constexpr auto SQL = "CREATE TABLE IF NOT EXISTS Users (Username TEXT PRIMARY KEY, Password TEXT);";
    char* err = nullptr;

    if (sqlite3_exec(database, SQL, nullptr, nullptr, &err) != SQLITE_OK) {
        logger.error(std::format("SQL Error: {}", err));
        sqlite3_free(err);
    }
}

std::string_view Database::check_identity(const std::string& username, const std::string& password)
{
    if (constexpr auto SQL = "SELECT Password FROM Users WHERE Username = ?";
        sqlite3_prepare_v2(database, SQL, -1, &cursor, nullptr) != SQLITE_OK)
        return ErrorMsg::SQL_INJECTION;

    sqlite3_bind_text(cursor, 1, username.c_str(), -1, SQLITE_STATIC);
    if (sqlite3_step(cursor) == SQLITE_ROW) {
        const auto stored_password = reinterpret_cast<const char*>(sqlite3_column_text(cursor, 0));
        return (password == stored_password) ? Status::SUCCESS : ErrorMsg::WRONG_PASSWORD;
    } else {
        return ErrorMsg::USER_NOT_FOUND;
    }
}
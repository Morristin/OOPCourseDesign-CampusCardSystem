#include "Database.h"

#include "../logger/Logger.h"

static auto logger = Logger(__FILE__);

Database::Database(const std::string& database_path)
{
    if (const int status = sqlite3_open(database_path.c_str(), &database); status != SQLITE_OK) {
        logger.critical(std::format("Can not connect to database: {}", database_path));
    }
}

void Database::initialize() const
{
    constexpr auto SQL = "CREATE TABLE IF NOT EXISTS Users ("
                         "Username TEXT PRIMARY KEY, "
                         "Password TEXT NOT NULL, "
                         "Permission INT NOT NULL, "
                         "Status TEXT NOT NULL, "
                         "CardNumber TEXT UNIQUE );";

    sqlite3_exec(database, SQL, nullptr, nullptr, nullptr);
}

LoginUserStatus Database::check_identity(const std::string& username, const std::string& password)
{
    if (constexpr auto SQL = "SELECT Password, Permission, Status, CardNumber FROM Users WHERE Username = ?";
        sqlite3_prepare_v2(database, SQL, -1, &cursor, nullptr) != SQLITE_OK) {
        logger.error(std::format("SQL Error: {}", sqlite3_errmsg(database)));
        throw DatabaseException(ErrorMsg::FINDING_FAILED);
    }

    sqlite3_bind_text(cursor, 1, username.c_str(), -1, SQLITE_STATIC);
    if (sqlite3_step(cursor) != SQLITE_ROW)
        throw DatabaseException(ErrorMsg::USER_NOT_FOUND);

    if (const auto stored_password = reinterpret_cast<const char*>(sqlite3_column_text(cursor, 0));
        password != stored_password)
        throw DatabaseException(ErrorMsg::WRONG_PASSWORD);

    const auto permission = reinterpret_cast<const char*>(sqlite3_column_text(cursor, 1));
    const auto status = reinterpret_cast<const char*>(sqlite3_column_text(cursor, 2));
    const auto card_number = reinterpret_cast<const char*>(sqlite3_column_text(cursor, 3));
    return { permission, status, card_number };
}
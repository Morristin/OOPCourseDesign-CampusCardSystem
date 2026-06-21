#include "Database.h"

#include "../logger/Logger.h"

static auto logger = Logger(__FILE__);

std::mutex Database::database_mutex;

Database::Database(const std::string& database_path)
{
    if (const int status = sqlite3_open(database_path.c_str(), &database); status != SQLITE_OK)
        logger.critical(std::format("Can not connect to database: {}", database_path));
    else
        this->initialize();
}

void Database::initialize() const
{
    constexpr auto SQL_CREATE_TABLE_USERS = "CREATE TABLE IF NOT EXISTS Users ("
                                            "Username TEXT PRIMARY KEY, "
                                            "Password TEXT NOT NULL, "
                                            "Permission INTEGER NOT NULL, "
                                            "Status INTEGER NOT NULL, "
                                            "CardNumber TEXT NOT NULL );";
    constexpr auto SQL_CREATE_TABLE_USERINFO = "CREATE TABLE IF NOT EXISTS UserInfo ("
                                               "Username TEXT PRIMARY KEY, "
                                               "RealName TEXT NOT NULL, "
                                               "Gender INTEGER NOT NULL, "
                                               "StudentID TEXT NOT NULL UNIQUE, "
                                               "Department TEXT NOT NULL );";
    constexpr auto SQL_CREATE_TABLE_TRANSACTIONS = "CREATE TABLE IF NOT EXISTS Transactions ("
                                                   "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
                                                   "CardNumber TEXT NOT NULL,"
                                                   "Amount REAL NOT NULL, "
                                                   "Balance REAL NOT NULL, "
                                                   "TransactionTime TEXT NOT NULL,"
                                                   "Operator TEXT );";

    sqlite3_exec(database, SQL_CREATE_TABLE_USERS, nullptr, nullptr, nullptr);
    sqlite3_exec(database, SQL_CREATE_TABLE_USERINFO, nullptr, nullptr, nullptr);
    sqlite3_exec(database, SQL_CREATE_TABLE_TRANSACTIONS, nullptr, nullptr, nullptr);
}

LoginUserStatus Database::check_identity(const std::string& username, const std::string& password)
{
    std::lock_guard<std::mutex> lock(database_mutex);

    if (constexpr auto SQL = "SELECT Password, Permission, Status, CardNumber FROM Users WHERE Username = ?";
        sqlite3_prepare_v2(database, SQL, -1, &cursor, nullptr) != SQLITE_OK) {
        logger.error(std::format("SQL Error: {}", sqlite3_errmsg(database)));
        throw DatabaseException(ErrorMsg::DATABASE_FIND_USER_FAILED);
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
    return { username, permission, status, card_number };
}

void Database::add_operator(const std::string& username, const std::string& password)
{
    std::lock_guard<std::mutex> lock(database_mutex);

    if (constexpr auto SQL = "INSERT INTO Users (Username, Password, Permission, Status, CardNumber) VALUES (?, ?, ?, ?, ?)"; sqlite3_prepare_v2(database, SQL, -1, &cursor, nullptr) != SQLITE_OK)
        logger.error(std::format("SQL Error: {}", sqlite3_errmsg(database)));

    sqlite3_bind_text(cursor, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(cursor, 2, password.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(cursor, 3, Permission::OPERATOR);
    sqlite3_bind_int(cursor, 4, UserStatus::NORMAL);
    sqlite3_bind_text(cursor, 5, "NULL", -1, SQLITE_STATIC);

    if (sqlite3_step(cursor) != SQLITE_DONE)
        throw DatabaseException(ErrorMsg::USER_ALREADY_EXISTS);

    logger.info(std::format("Successfully added new operator {} with password {}", username, password));
}

void Database::del_operator(const std::string& username)
{
    std::lock_guard<std::mutex> lock(database_mutex);

    if (constexpr auto SQL = "DELETE FROM Users WHERE Username = ? AND Permission = ?"; sqlite3_prepare_v2(database, SQL, -1, &cursor, nullptr) != SQLITE_OK)
        logger.error(std::format("SQL Error: {}", sqlite3_errmsg(database)));

    sqlite3_bind_text(cursor, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(cursor, 2, Permission::OPERATOR);

    // This is actually a try-catch like code block.
    // If database does not delete anything, then throw USER_NOT_FOUND error.
    if (sqlite3_step(cursor) != SQLITE_DONE)
        throw DatabaseException(ErrorMsg::DATABASE_FIND_USER_FAILED);
    if (sqlite3_changes(database) == 0)
        throw DatabaseException(ErrorMsg::USER_NOT_FOUND);

    logger.info(std::format("Successfully deleted operator {}", username));
}
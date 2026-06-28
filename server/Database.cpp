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
    constexpr auto SQL_CREATE_TABLE_USERS        = "CREATE TABLE IF NOT EXISTS Users ("
                                                   "Username TEXT PRIMARY KEY, "
                                                   "Password TEXT NOT NULL, "
                                                   "Permission INTEGER NOT NULL, "
                                                   "Status INTEGER NOT NULL, "
                                                   "CardNumber TEXT NOT NULL );";
    constexpr auto SQL_CREATE_TABLE_USERINFO     = "CREATE TABLE IF NOT EXISTS UserInfo ("
                                                   "Username TEXT PRIMARY KEY, "
                                                   "RealName TEXT NOT NULL, "
                                                   "Gender TEXT NOT NULL, "
                                                   "StudentID TEXT NOT NULL UNIQUE, "
                                                   "Department TEXT NOT NULL );";
    constexpr auto SQL_CREATE_TABLE_TRANSACTIONS = "CREATE TABLE IF NOT EXISTS Transactions ("
                                                   "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
                                                   "CardNumber TEXT NOT NULL,"
                                                   "Amount REAL NOT NULL, "
                                                   "Balance REAL NOT NULL, "
                                                   "TransactionTime TEXT NOT NULL,"
                                                   "Operator TEXT NOT NULL );";

    sqlite3_exec(database, SQL_CREATE_TABLE_USERS, nullptr, nullptr, nullptr);
    sqlite3_exec(database, SQL_CREATE_TABLE_USERINFO, nullptr, nullptr, nullptr);
    sqlite3_exec(database, SQL_CREATE_TABLE_TRANSACTIONS, nullptr, nullptr, nullptr);

    constexpr auto SQL_CREATE_TABLE_SYSTEM_SETTINGS = "CREATE TABLE IF NOT EXISTS SystemSettings ("
                                                      "Key TEXT PRIMARY KEY, "
                                                      "Value REAL NOT NULL);";
    constexpr auto SQL_INIT_TABLE_SYSTEM_SETTINGS   = "INSERT OR IGNORE INTO SystemSettings (Key, Value) "
                                                      "VALUES ('FIXED_FEE', 0.0)";

    sqlite3_exec(database, SQL_CREATE_TABLE_SYSTEM_SETTINGS, nullptr, nullptr, nullptr);
    sqlite3_exec(database, SQL_INIT_TABLE_SYSTEM_SETTINGS, nullptr, nullptr, nullptr);
}

void Database::check_and_deduct_fixed_fee()
{
    std::lock_guard lock(database_mutex);

    // Check the last deduction time. If the data doesn't match requirment, just return.
    constexpr auto FIXED_FEE_OPERATOR            = "SYSTEM_FEE";
    constexpr auto SQL_CHECK_LAST_DEDUCTION_TIME = "SELECT COUNT(*) FROM Transactions WHERE Transactions.Operator = ? AND TransactionTime > datetime('now', '-180 days')";

    sqlite3_prepare_v2(database, SQL_CHECK_LAST_DEDUCTION_TIME, -1, &cursor, nullptr);
    sqlite3_bind_text(cursor, 1, FIXED_FEE_OPERATOR, -1, SQLITE_STATIC);

    if (sqlite3_step(cursor); sqlite3_column_int(cursor, 0) != 0)
        return;

    // If the deduction never happened, do dudect at 3.1, 6.1, 9.1 and 12.1.
    // Detect date from database instead of server.
    constexpr auto SQL_CHECK_DEDUCTION_EVER_HAPPENED = "SELECT COUNT(*) FROM Transactions WHERE Operator = ?";

    sqlite3_prepare_v2(database, SQL_CHECK_DEDUCTION_EVER_HAPPENED, -1, &cursor, nullptr);
    sqlite3_bind_text(cursor, 1, FIXED_FEE_OPERATOR, -1, SQLITE_STATIC);

    sqlite3_prepare_v2(database, "SELECT strftime('%m-%d', 'now')", -1, &cursor, nullptr);
    sqlite3_step(cursor);

    if (const std::string today = reinterpret_cast<const char*>(sqlite3_column_text(cursor, 0));
        today != "03-01" && today != "06-01" && today != "09-01" && today != "12-01")
        return;

    // Get all card for deducting fixed fee from table Users.
    std::vector<std::string> card_numbers;

    constexpr auto SQL_GET_ALL_CARDNUMBER = "SELECT Users.CardNumber FROM Users "
                                            "WHERE Users.Status = ? AND Users.CardNumber IS NOT NULL";

    sqlite3_prepare_v2(database, SQL_GET_ALL_CARDNUMBER, -1, &cursor, nullptr);
    sqlite3_bind_int(cursor, 1, UserStatus::NORMAL);

    while (sqlite3_step(cursor) == SQLITE_ROW) {
        auto card_number = reinterpret_cast<const char*>(sqlite3_column_text(cursor, 0));
        card_numbers.emplace_back(card_number);
    }

    // Get the fixed fee from table System Settings
    double fixed_fee = 0.0;

    sqlite3_prepare_v2(database, "SELECT Value FROM SystemSettings WHERE Key = 'SEMESTER_FEE'", -1, &cursor, nullptr);
    if (sqlite3_step(cursor) == SQLITE_ROW)
        fixed_fee = sqlite3_column_double(cursor, 1);
    else
        logger.error("Cannot detect FIXED_FEE from table SystemSettings.");

    // Do every change in transaction and commit in the end.
    sqlite3_exec(database, "BEGIN TRANSACTION", nullptr, nullptr, nullptr);

    for (const auto& card_number : card_numbers) {
        try {
            consume_card(card_number, fixed_fee, "FIXED_FEE");
        } catch (const DatabaseException& err) {
            logger.warning(std::format("Failed to deduct fixed fee for card {}: {}", card_number, err.what()));
        }
    }

    sqlite3_exec(database, "COMMIT", nullptr, nullptr, nullptr);
    logger.info(std::format("Successfully deducted semester fee from {} accounts.", card_numbers.size()));
}

std::string Database::query_account(const std::string& username)
{
    std::lock_guard lock(database_mutex);

    constexpr auto SQL = "SELECT Password, Permission, Status, CardNumber FROM Users WHERE Username = ?";
    sqlite3_prepare_v2(database, SQL, -1, &cursor, nullptr);

    sqlite3_bind_text(cursor, 1, username.c_str(), -1, SQLITE_STATIC);
    if (sqlite3_step(cursor) != SQLITE_ROW)
        throw DatabaseException(ErrorMsg::USER_NOT_FOUND);

    std::string password    = reinterpret_cast<const char*>(sqlite3_column_text(cursor, 0));
    std::string permission  = reinterpret_cast<const char*>(sqlite3_column_text(cursor, 1));
    std::string status      = reinterpret_cast<const char*>(sqlite3_column_text(cursor, 2));
    std::string card_number = reinterpret_cast<const char*>(sqlite3_column_text(cursor, 3));

    return std::format(DB_USER_INFO, password, permission, status, card_number);
}

void Database::create_account(const std::string& username, const std::string& password, int permission)
{
    std::lock_guard lock(database_mutex);

    constexpr auto SQL = "INSERT INTO Users (Username, Password, Permission, Status, CardNumber) VALUES (?, ?, ?, ?, ?)";
    sqlite3_prepare_v2(database, SQL, -1, &cursor, nullptr);

    sqlite3_bind_text(cursor, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(cursor, 2, password.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(cursor, 3, permission);
    sqlite3_bind_int(cursor, 4, UserStatus::NORMAL);
    sqlite3_bind_text(cursor, 5, CardNumber::BLANK.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(cursor) != SQLITE_DONE)
        throw DatabaseException(ErrorMsg::USER_EXISTS);

    logger.info(std::format("Successfully added new operator {} with password {}", username, password));
}

void Database::update_account_status(const std::string& username, const int new_status)
{
    std::lock_guard lock(database_mutex);

    // Check whether the username exist, and whether the user is student.
    sqlite3_prepare_v2(database, "SELECT Permission FROM Users WHERE Username = ?", -1, &cursor, nullptr);
    sqlite3_bind_text(cursor, 1, username.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(cursor) != SQLITE_ROW)
        throw DatabaseException(ErrorMsg::USER_NOT_FOUND);
    if (const int target_permission = sqlite3_column_int(cursor, 0); target_permission != Permission::STUDENT)
        throw DatabaseException(ErrorMsg::TARGET_NOT_STUDENT);

    // Update the status of the specific user.
    sqlite3_prepare_v2(database, "UPDATE Users SET Status = ? WHERE Username = ?", -1, &cursor, nullptr);
    sqlite3_bind_int(cursor, 1, new_status);
    sqlite3_bind_text(cursor, 2, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_step(cursor);
}

std::vector<std::string> Database::query_abnormal_accounts()
{
    std::vector<std::string> records;

    constexpr auto SQL = "SELECT Username, CardNumber, Status FROM Users WHERE Status != ?";
    sqlite3_prepare_v2(database, SQL, -1, &cursor, nullptr);
    sqlite3_bind_int(cursor, 1, UserStatus::NORMAL);

    while (sqlite3_step(cursor) == SQLITE_ROW) {
        std::string username    = reinterpret_cast<const char*>(sqlite3_column_text(cursor, 0));
        std::string card_number = reinterpret_cast<const char*>(sqlite3_column_text(cursor, 1));
        const int origin_status = sqlite3_column_int(cursor, 2);

        std::string status;
        if (origin_status == UserStatus::FROZEN)
            status = "Frozen";
        else if (origin_status == UserStatus::DELETED)
            status = "Deleted";
        records.emplace_back(std::format(DB_ABNORMAL_ACCOUNT, username, card_number, status));
    }

    return records;
}

void Database::delete_operator(const std::string& username)
{
    std::lock_guard lock(database_mutex);

    constexpr auto SQL = "DELETE FROM Users WHERE Username = ? AND Permission = ?";
    sqlite3_prepare_v2(database, SQL, -1, &cursor, nullptr);
    sqlite3_bind_text(cursor, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(cursor, 2, Permission::OPERATOR);

    // This is actually a try-catch like code block.
    // If database does not delete anything, then throw USER_NOT_FOUND error.
    if (sqlite3_step(cursor); sqlite3_changes(database) == 0)
        throw DatabaseException(ErrorMsg::USER_NOT_FOUND);

    logger.info(std::format("Successfully deleted operator {}", username));
}

void Database::create_student(const std::string& real_name, const std::string& gender, const std::string& student_id, const std::string& department)
{
    std::lock_guard lock(database_mutex);

    sqlite3_prepare_v2(database, "INSERT INTO UserInfo (Username, RealName, Gender, StudentID, Department) VALUES (?, ?, ?, ?, ?)", -1, &cursor, nullptr);
    sqlite3_bind_text(cursor, 1, student_id.c_str(), -1, SQLITE_STATIC); // Use StudentID as the default username.
    sqlite3_bind_text(cursor, 2, real_name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(cursor, 3, gender.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(cursor, 4, student_id.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(cursor, 5, department.c_str(), -1, SQLITE_STATIC);

    // If student ID already exist, throw exception.
    if (sqlite3_step(cursor) != SQLITE_DONE)
        throw DatabaseException(ErrorMsg::USERINFO_EXISTS);

    try {
        create_account(student_id, Password::DEFAULT, Permission::STUDENT);
    } catch (const DatabaseException&) {
        // If create_account failed，rollback to previous UserInfo table.
        sqlite3_prepare_v2(database, "DELETE FROM UserInfo WHERE Username = ?", -1, &cursor, nullptr);
        sqlite3_bind_text(cursor, 1, student_id.c_str(), -1, SQLITE_STATIC);
        sqlite3_step(cursor);
        throw;
    }
}

void Database::delete_student(const std::string& student_id)
{
    std::lock_guard lock(database_mutex);

    sqlite3_prepare_v2(database, "SELECT Username FROM UserInfo WHERE Username = ?", -1, &cursor, nullptr);
    sqlite3_bind_text(cursor, 1, student_id.c_str(), -1, SQLITE_STATIC);
    if (sqlite3_step(cursor) != SQLITE_ROW)
        throw DatabaseException(ErrorMsg::USERINFO_NOT_FOUND);

    // Only set the user in User table to DELETED status, so the data can be recovered easily.
    const auto username = reinterpret_cast<const char*>(sqlite3_column_text(cursor, 0));
    update_account_status(username, UserStatus::DELETED);

    sqlite3_prepare_v2(database, "DELETE FROM UserInfo WHERE Username = ?", -1, &cursor, nullptr);
    sqlite3_bind_text(cursor, 1, student_id.c_str(), -1, SQLITE_STATIC);
    sqlite3_step(cursor);
}

void Database::update_student(const std::string& student_id, const std::string& real_name, const std::string& gender, const std::string& department)
{
    std::lock_guard lock(database_mutex);

    sqlite3_prepare_v2(database, "SELECT Username FROM UserInfo WHERE Username = ?", -1, &cursor, nullptr);
    sqlite3_bind_text(cursor, 1, student_id.c_str(), -1, SQLITE_STATIC);
    if (sqlite3_step(cursor) != SQLITE_ROW)
        throw DatabaseException(ErrorMsg::USERINFO_NOT_FOUND);

    // Use COALESCE and NULLIF to only update specific parts of UserInfo.
    constexpr auto SQL = "UPDATE UserInfo SET "
                         "RealName = COALESCE(NULLIF(?, ''), RealName), "
                         "Gender = COALESCE(NULLIF(?, ''), Gender), "
                         "Department = COALESCE(NULLIF(?, ''), Department) "
                         "WHERE Username = ?";

    sqlite3_prepare_v2(database, SQL, -1, &cursor, nullptr);
    sqlite3_bind_text(cursor, 1, real_name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(cursor, 2, gender.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(cursor, 3, department.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(cursor, 4, student_id.c_str(), -1, SQLITE_STATIC);

    sqlite3_step(cursor);
}

void Database::recharge_card(const std::string& card_number, double amount, const std::string& operator_name)
{
    std::lock_guard lock(database_mutex);

    // Check whether the card number exist and account is in normal status.
    sqlite3_prepare_v2(database, "SELECT Status FROM Users WHERE CardNumber = ?", -1, &cursor, nullptr);
    sqlite3_bind_text(cursor, 1, card_number.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(cursor) != SQLITE_ROW)
        throw DatabaseException(ErrorMsg::CARD_NOT_FOUND);
    if (sqlite3_column_int(cursor, 0) != UserStatus::NORMAL)
        throw DatabaseException(ErrorMsg::ACCOUNT_ABNORMAL);

    // Get current balance from table Transactions. If there is no record in Transactions, use default value 0.
    double old_balance     = 0.0;
    constexpr auto SQL_GET = "SELECT Balance FROM Transactions WHERE CardNumber = ? ORDER BY ID DESC LIMIT 1";
    sqlite3_prepare_v2(database, SQL_GET, -1, &cursor, nullptr);
    sqlite3_bind_text(cursor, 1, card_number.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(cursor) == SQLITE_ROW)
        old_balance = sqlite3_column_double(cursor, 0);

    // Calculate new balance and insert recharge record into Transactions.
    const double new_balance  = old_balance + amount;
    constexpr auto SQL_INSERT = "INSERT INTO Transactions (CardNumber, Amount, Balance, TransactionTime, Operator) "
                                "VALUES (?, ?, ?, datetime('now', 'localtime'), ?)";
    sqlite3_prepare_v2(database, SQL_INSERT, -1, &cursor, nullptr);
    sqlite3_bind_text(cursor, 1, card_number.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(cursor, 2, amount);
    sqlite3_bind_double(cursor, 3, new_balance);
    sqlite3_bind_text(cursor, 4, operator_name.c_str(), -1, SQLITE_STATIC);

    sqlite3_step(cursor);
}

void Database::consume_card(const std::string& card_number, const double amount, const std::string& merchant)
{
    std::lock_guard lock(database_mutex);

    // Check whether the card number exist and account is in normal status.
    sqlite3_prepare_v2(database, "SELECT Status FROM Users WHERE CardNumber = ?", -1, &cursor, nullptr);
    sqlite3_bind_text(cursor, 1, card_number.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(cursor) != SQLITE_ROW)
        throw DatabaseException(ErrorMsg::CARD_NOT_FOUND);
    if (sqlite3_column_int(cursor, 0) != UserStatus::NORMAL)
        throw DatabaseException(ErrorMsg::ACCOUNT_ABNORMAL);

    // Get current balance from table Transactions. Check whether the balance is greater than amount.
    double current_balance = 0.0;
    sqlite3_prepare_v2(database, "SELECT Balance FROM Transactions WHERE CardNumber = ? ORDER BY ID DESC LIMIT 1", -1, &cursor, nullptr);
    sqlite3_bind_text(cursor, 1, card_number.c_str(), -1, SQLITE_STATIC);
    if (sqlite3_step(cursor) == SQLITE_ROW)
        current_balance = sqlite3_column_double(cursor, 0);

    if (current_balance < amount)
        throw DatabaseException(ErrorMsg::BALANCE_INSUFFICIENT);

    // Calculate new balance and insert consume record into Transactions.
    const double new_balance = current_balance - amount;
    sqlite3_prepare_v2(database, "INSERT INTO Transactions (CardNumber, Amount, Balance, TransactionTime, Operator) VALUES (?, ?, ?, datetime('now', 'localtime'), ?)", -1, &cursor, nullptr);
    sqlite3_bind_text(cursor, 1, card_number.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(cursor, 2, -amount);
    sqlite3_bind_double(cursor, 3, new_balance);
    sqlite3_bind_text(cursor, 4, merchant.c_str(), -1, SQLITE_STATIC);

    sqlite3_step(cursor);
}

std::vector<std::string> Database::query_transactions(const std::string& card_number)
{
    std::lock_guard lock(database_mutex);

    sqlite3_prepare_v2(database, "SELECT TransactionTime, Amount, Balance, Operator FROM Transactions WHERE CardNumber = ? ORDER BY ID DESC", -1, &cursor, nullptr);
    sqlite3_bind_text(cursor, 1, card_number.c_str(), -1, SQLITE_STATIC);

    std::vector<std::string> records;
    while (sqlite3_step(cursor) == SQLITE_ROW) {
        auto time    = reinterpret_cast<const char*>(sqlite3_column_text(cursor, 0));
        auto amount  = sqlite3_column_double(cursor, 1);
        auto balance = sqlite3_column_double(cursor, 2);
        auto op      = reinterpret_cast<const char*>(sqlite3_column_text(cursor, 3));

        records.emplace_back(std::format(DB_TRANSACTION_RECORD, time, std::format("{:.2f}", amount), std::format("{:.2f}", balance), op));
    }

    if (records.empty())
        throw DatabaseException(ErrorMsg::TRANSACTION_NOT_FOUND);
    return records;
}

std::vector<std::string> Database::query_merchant_transactions(const std::string& merchant)
{
    std::lock_guard lock(database_mutex);

    constexpr auto SQL = "SELECT TransactionTime, Amount, Balance, Operator FROM Transactions WHERE Operator = ? ORDER BY TransactionTime DESC";
    sqlite3_prepare_v2(database, SQL, -1, &cursor, nullptr);
    sqlite3_bind_text(cursor, 1, merchant.c_str(), -1, SQLITE_STATIC);

    std::vector<std::string> records;
    while (sqlite3_step(cursor) == SQLITE_ROW) {
        std::string time = reinterpret_cast<const char*>(sqlite3_column_text(cursor, 0));
        double amount    = sqlite3_column_double(cursor, 1);
        double balance   = sqlite3_column_double(cursor, 2);

        records.emplace_back(std::format(DB_TRANSACTION_RECORD, time, std::format("{:.2f}", amount), std::format("{:.2f}", balance), merchant));
    }

    return records;
}

std::vector<std::string> Database::export_transactions()
{
    std::lock_guard lock(database_mutex);
    std::vector<std::string> records;

    constexpr auto SQL = "SELECT TransactionTime, Amount, Balance, Operator FROM Transactions ORDER BY TransactionTime DESC";
    sqlite3_prepare_v2(database, SQL, -1, &cursor, nullptr);

    while (sqlite3_step(cursor) == SQLITE_ROW) {
        std::string time = reinterpret_cast<const char*>(sqlite3_column_text(cursor, 0));
        double amount    = sqlite3_column_double(cursor, 1);
        double balance   = sqlite3_column_double(cursor, 2);
        std::string op   = reinterpret_cast<const char*>(sqlite3_column_text(cursor, 3));

        records.emplace_back(std::format(DB_TRANSACTION_RECORD, time, std::format("{:.2f}", amount), std::format("{:.2f}", balance), op));
    }

    return records;
}

std::vector<std::string> Database::generate_statistics(const std::string& type)
{
    std::vector<std::string> records;
    std::string sql;

    constexpr std::string_view department_SQL = "SELECT UserInfo.Department, SUM(ABS(Transactions.Amount)), COUNT(Transactions.ID) "
                                                "FROM Transactions JOIN Users ON Transactions.CardNumber = Users.CardNumber JOIN UserInfo ON Users.Username = UserInfo.Username "
                                                "WHERE Transactions.Amount < 0 "
                                                "GROUP BY UserInfo.Department";
    constexpr std::string_view merchant_SQL   = "SELECT Transactions.Operator, SUM(ABS(Transactions.Amount)), COUNT(Transactions.ID) "
                                                "FROM Transactions "
                                                "WHERE Transactions.Amount < 0 "
                                                "GROUP BY Transactions.Operator";
    constexpr std::string_view time_SQL       = "SELECT strftime('%Y-%m', Transactions.TransactionTime), SUM(Transactions.Amount), COUNT(Transactions.ID) "
                                                "FROM Transactions "
                                                "WHERE Transactions.Amount < 0 "
                                                "GROUP BY strftime('%Y-%m', Transactions.TransactionTime)";
    constexpr std::string_view ranking_SQL    = "SELECT UserInfo.RealName, SUM(ABS(Transactions.Amount)), COUNT(Transactions.ID) "
                                                "FROM Transactions JOIN Users ON Transactions.CardNumber = Users.CardNumber JOIN UserInfo ON Users.Username = UserInfo.Username "
                                                "WHERE Transactions.Amount < 0 "
                                                "GROUP BY Users.Username "
                                                "ORDER BY SUM(ABS(Transactions.Amount)) DESC "
                                                "LIMIT 10";

    if (type == "department")
        sql = department_SQL;
    else if (type == "merchant")
        sql = merchant_SQL;
    else if (type == "time")
        sql = time_SQL;
    else if (type == "ranking")
        sql = ranking_SQL;

    sqlite3_prepare_v2(database, sql.c_str(), -1, &cursor, nullptr);

    while (sqlite3_step(cursor) == SQLITE_ROW) {
        std::string category = reinterpret_cast<const char*>(sqlite3_column_text(cursor, 0));
        double consumption   = sqlite3_column_double(cursor, 1);
        int count            = sqlite3_column_int(cursor, 2);
        records.emplace_back(std::format(DB_TRANSACTION_STATISTICS, category, std::format("{:.2f}", consumption), count));
    }

    return records;
}
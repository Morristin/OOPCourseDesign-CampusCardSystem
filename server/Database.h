#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include <string>

#include "../protocol/protocol.h"

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

    void create_account(const std::string& username, const std::string& password, int permission);
    void del_operator(const std::string& username);

    void register_student(const std::string& real_name, const std::string& gender, const std::string& student_id, const std::string& department);
    void delete_student(const std::string& student_id);
    void update_student(const std::string& student_id, const std::string& real_name, const std::string& gender, const std::string& department);

    void recharge_card(const std::string& card_number, double amount, const std::string& operator_name);
    void update_account_status(const std::string& username, int new_status);

    void consume_card(const std::string& card_number, double amount, const std::string& merchant);
    std::vector<std::string> query_transactions(const std::string& card_number);
};

#endif

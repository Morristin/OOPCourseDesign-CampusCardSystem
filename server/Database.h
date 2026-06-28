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
    void update_system_setting(const std::string& key, const std::string& value);

    void check_and_deduct_fixed_fee();

    std::string query_account(const std::string& username);
    void create_account(const std::string& username, const std::string& password, int permission);
    void delete_operator(const std::string& username);
    void update_account_status(const std::string& username, int new_status);
    std::vector<std::string> query_abnormal_accounts();

    void create_student(const std::string& real_name, const std::string& gender, const std::string& student_id, const std::string& department);
    void delete_student(const std::string& student_id);
    void update_student(const std::string& student_id, const std::string& real_name, const std::string& gender, const std::string& department);

    void recharge_card(const std::string& card_number, double amount, const std::string& operator_name);
    void consume_card(const std::string& card_number, double amount, const std::string& merchant);
    std::vector<std::string> query_transactions(const std::string& card_number);
    std::vector<std::string> query_merchant_transactions(const std::string& merchant);
    std::vector<std::string> export_transactions();
    std::vector<std::string> generate_statistics(const std::string& type);
};

#endif

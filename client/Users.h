#ifndef USERS_H
#define USERS_H

#include <utility>

#include "Client.h"
#include "Dashboard.h"

struct UserInformation {
    const std::string username;
    const int permission;
    const int status;
    const std::string card_number;

    explicit UserInformation(const Parser& user_info_parser)
        : username(user_info_parser["username"]),
          permission(std::stoi(user_info_parser["permission"])),
          status(std::stoi(user_info_parser["user_status"])),
          card_number(user_info_parser["card_number"]) { };
};

class User {
protected:
    const Client& client;
    const UserInformation user_information;
    Dashboard dashboard = { };
    [[nodiscard]] virtual Dashboard main_dashboard() const { return dashboard; }

public:
    User(const Client& client, UserInformation user_information) : client(client), user_information(std::move(user_information)) { };
    virtual ~User() = default;
    void show_dashboard() const;
};

class Student : public User {
protected:
    [[nodiscard]] Dashboard main_dashboard() const override { return student_dashboard; }
    Dashboard student_dashboard = {
        MenuItem("Consume", "Consume the money with the info provided by merchant.", [this] { consume(); }),
        MenuItem("Query Transactions", "Query your transaction history.", [this] { query_own_records(); })
    };

    void consume() const;
    void query_own_records() const;

public:
    Student(const Client& client, UserInformation user_information) : User(client, std::move(user_information)) { };
};

class Operator : public User {
protected:
    [[nodiscard]] Dashboard main_dashboard() const override { return operator_dashboard; }

    Dashboard student_management_dashboard = {
        MenuItem("Create Student", "Add student account with student information.", [this] { create_student(); }),
        MenuItem("Import Students", "Create multiple student accounts using CSV.", [this] { create_multiple_student(); }),
        MenuItem("Delete Student", "Delete existing student account with student ID.", [this] { delete_student(); }),
        MenuItem("Modify Student Info", "Modify existing student info.", [this] { update_student_userinfo(); }),
        MenuItem("Query Abnormal Accounts", "List the frozen or deleted student account.", [this] { query_abnormal_accounts(); })
    };

    Dashboard transaction_management_dashboard = {
        MenuItem("Query Transactions", "Query the transaction history of a card.", [this] { query_transactions(); }),
        MenuItem("Query Merchant Transactions", "Query the transaction history of a merchant.", [this] { query_merchant_transactions(); }),
        MenuItem("Export Transactions", "Export all transactions into specific CSV file.", [this] { export_transaction(); }),
        MenuItem("Generate statistics", "Generate statistics group by department, merchant, time or person.", [this] { generate_statistics(); })
    };

    Dashboard operator_dashboard = {
        MenuItem("Manage Student Information", "Create, modify or delete student account.", &student_management_dashboard),
        MenuItem("Manage Student Accounts", "Freeze, delete or restore student account.", [this] { update_student_status(); }),
        MenuItem("Recharge", "Recharge money into a card.", [this] { recharge_card(); }),
        MenuItem("Set fixed fee", "set the value of fixed fee.", [this] { update_fixed_fee(); }),
        MenuItem("Manage Transactions", "Query or export transactions", &transaction_management_dashboard)
    };

    void update_fixed_fee() const;

    void create_student() const;
    void create_multiple_student() const;
    void delete_student() const;
    void update_student_userinfo() const;

    void recharge_card() const;
    void update_student_status() const;
    void query_abnormal_accounts() const;
    void query_transactions() const;
    void query_merchant_transactions() const;
    void export_transaction() const;
    void generate_statistics() const;

public:
    Operator(const Client& client, UserInformation user_information) : User(client, std::move(user_information)) { };
};

class SuperOperator : public Operator {
protected:
    [[nodiscard]] Dashboard main_dashboard() const override { return superoperator_dashboard; }
    Dashboard superoperator_dashboard = {
        MenuItem("Create Operator", "Create an operator.", [this] { create_operator(); }),
        MenuItem("Delete Operator", "Delete an operator.", [this] { delete_operator(); }),
        MenuItem("Reset Operator Password", "Reset an operator's password.", [this] { reset_operator_password(); }),
        MenuItem("Export Server Logs", "export server logs to local file.", [this] { export_system_logs(); }),
        MenuItem("Switch to Operator Dashboard", "Enter the operator view. (Using 'b' to return here)", &operator_dashboard)
    };

    void create_operator() const;
    void delete_operator() const;
    void reset_operator_password() const;
    void export_system_logs() const;

public:
    SuperOperator(const Client& client, UserInformation user_information) : Operator(client, std::move(user_information)) { };
};

#endif

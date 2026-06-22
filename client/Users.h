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
    [[nodiscard]] Dashboard main_dashboard() const override { return dashboard; }
    Dashboard dashboard = {
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
    [[nodiscard]] Dashboard main_dashboard() const override { return dashboard; }

    Dashboard student_management = {
        MenuItem("Create Student", "Add student account with student information.", [this] { create_student(); }),
        MenuItem("Import Students", "Create multiple student accounts using CSV.", [this] { create_multiple_student(); }),
        MenuItem("Delete Student", "Delete existing student account with student ID.", [this] { delete_student(); }),
        MenuItem("Modify Student Info", "Modify existing student info.", [this] { update_student_userinfo(); }),
        MenuItem("Query Abnormal Accounts", "List the frozen or deleted student account.", [this] { query_abnormal_accounts(); })
    };

    Dashboard dashboard = {
        MenuItem("Manage Student Information", "Create, modify or delete student account.", &student_management),
        MenuItem("Manage Student Accounts", "Freeze, delete or restore student account.", [this] { update_student_status(); }),
        MenuItem("Recharge", "Recharge money into a card.", [this] { recharge_card(); }),
        MenuItem("Query Transactions", "Query the transaction history of a card.", [this] { query_transactions(); })
    };

    void create_student() const;
    void create_multiple_student() const;
    void delete_student() const;
    void update_student_userinfo() const;

    void recharge_card() const;
    void update_student_status() const;
    void query_abnormal_accounts() const;
    void query_transactions() const;

public:
    Operator(const Client& client, UserInformation user_information) : User(client, std::move(user_information)) { };
};

class SuperOperator : public Operator {
protected:
    [[nodiscard]] Dashboard main_dashboard() const override { return dashboard; }
    Dashboard dashboard = {
        MenuItem("Create Operator", "Create an operator.", [this] { create_operator(); }),
        MenuItem("Delete Operator", "Delete an operator.", [this] { delete_operator(); }),
        MenuItem("Reset Operator Password", "Reset an operator's password.", [this] { reset_operator_password(); })
    };

    void create_operator() const;
    void delete_operator() const;
    void reset_operator_password() const;

public:
    SuperOperator(const Client& client, UserInformation user_information) : Operator(client, std::move(user_information)) { };
};

#endif

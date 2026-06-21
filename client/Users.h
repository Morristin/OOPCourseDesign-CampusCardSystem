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

    explicit UserInformation(const Parser& user_info_parser) : username(user_info_parser["username"]), permission(std::stoi(user_info_parser["permission"])), status(std::stoi(user_info_parser["user_status"])), card_number(user_info_parser["card_number"]) { };
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
    void consume() const;

    [[nodiscard]] Dashboard main_dashboard() const override { return dashboard; }
    Dashboard dashboard = {
        MenuItem("Consume", "Consume the money with the amount and merchant provided by money receiver.", [this] { consume(); }),
    };

public:
    Student(const Client& client, UserInformation user_information) : User(client, std::move(user_information)) { };
};

class Operator : public User {
protected:
    void add_student() const;
    void add_multiple_student() const;
    void del_student() const;
    void update_student() const;

    void recharge() const;
    void manage_accounts() const;

    [[nodiscard]] Dashboard main_dashboard() const override { return dashboard; }

    Dashboard student_management = {
        MenuItem("Add Student", "Add student account with student information.", [this] { add_student(); }),
        MenuItem("Import Students", "Create multiple student accounts using CSV.", [this] { add_multiple_student(); }),
        MenuItem("Delete Student", "Delete existing student account with student ID.", [this] { del_student(); }),
        MenuItem("Modify Student Info", "Modify existing student info.", [this] { update_student(); }),
    };

    Dashboard dashboard = {
        MenuItem("Manage Student Information", "Add, modify and delete student account.", &student_management),
        MenuItem("Manage Student Accounts", "Freeze, delete or restore accounts.", [this] { manage_accounts(); }),
        MenuItem("Recharge", "Recharge money into specific card.", [this] { recharge(); }),
    };

public:
    Operator(const Client& client, UserInformation user_information) : User(client, std::move(user_information)) { };
};

class SuperOperator : public Operator {
protected:
    void add_operator() const;
    void delete_operator() const;
    void reset_operator_password() const;

    [[nodiscard]] Dashboard main_dashboard() const override { return dashboard; }
    Dashboard dashboard = {
        MenuItem("Add Operator", "Add an operator.", [this] { add_operator(); }),
        MenuItem("Delete Operator", "Delete a exist operator.", [this] { delete_operator(); }),
        MenuItem("Reset Operator Password", "Reset a exist operator's password.", [this] { delete_operator(); }),
    };

public:
    SuperOperator(const Client& client, UserInformation user_information) : Operator(client, std::move(user_information)) { };
};

#endif

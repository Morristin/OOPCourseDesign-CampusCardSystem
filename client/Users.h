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

public:
    User(const Client& client, UserInformation user_information) : client(client), user_information(std::move(user_information)) { };
    virtual ~User() = default;

    virtual void show_dashboard();
};

class Student : public User {
public:
    Student(const Client& client, UserInformation user_information) : User(client, std::move(user_information)) { };
};

class Operator : public User {
public:
    Operator(const Client& client, UserInformation user_information) : User(client, std::move(user_information)) { };
};

class SuperOperator : public User {
protected:
    void add_operator() const;
    void delete_operator() const;
    void reset_operator_password() const;

public:
    SuperOperator(const Client& client, UserInformation user_information) : User(client, std::move(user_information))
    {
        dashboard = {
            MenuItem("Add Operator", "Add an operator.", [this] { add_operator(); }),
            MenuItem("Delete Operator", "Delete a exist operator.", [this] { delete_operator(); }),
            MenuItem("Reset Operator Password", "Reset a exist operator's password.", [this] { delete_operator(); }),
        };
    };
};

#endif

#ifndef USERS_H
#define USERS_H

#include <utility>

#include "Client.h"

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

public:
    User(const Client& client, UserInformation user_information) : client(client), user_information(std::move(user_information)) { };
    virtual ~User() = default;

    virtual int dashboard();
};

class Student : public User {
public:
    Student(const Client& client, UserInformation user_information) : User(client, std::move(user_information)) { };
    int dashboard() override;
};

class Operator : public User {
public:
    Operator(const Client& client, UserInformation user_information) : User(client, std::move(user_information)) { };
    int dashboard() override;
};

class SuperOperator : public User {
public:
    SuperOperator(const Client& client, UserInformation user_information) : User(client, std::move(user_information)) { };
    int dashboard() override;
};

#endif

#include "../logger/Logger.h"
#include "../stream/protocol.h"
#include "Users.h"

#include <iostream>

static auto logger = Logger(__FILE__);

void SuperOperator::add_operator() const
{
    std::string username, password;
    std::cout << "Please enter new operator's username and password: " << std::endl;
    std::cin >> username >> password;

    client.send_msg(std::format(ACTION_ADD_OPERATOR, username, password));
    const auto response = client.receive_msg();

    if (response["status"] == MsgStatus::SUCCESS)
        std::cout << "Operator added successfully." << std::endl;
    else if (response["message"] == ErrorMsg::USER_ALREADY_EXISTS)
        std::cout << "Failed to add operator as username already exist. Please change one and try again." << std::endl;
}

void SuperOperator::delete_operator() const
{
    std::string username;
    std::cout << "Please enter the username of the operator to delete: " << std::endl;
    std::cin >> username;

    client.send_msg(std::format(ACTION_DELETE_OPERATOR, username));
    const auto response = client.receive_msg();

    if (response["status"] == MsgStatus::SUCCESS)
        std::cout << "Operator deleted successfully." << std::endl;
    else if (response["message"] == ErrorMsg::USER_NOT_FOUND)
        std::cout << "Failed to delete operator as the username does not exist." << std::endl;
}

void SuperOperator::reset_operator_password() const
{
    std::string username, new_password;
    std::cout << "Enter the operator's username and new password: " << std::endl;
    std::cin >> username >> new_password;

    client.send_msg(std::format(ACTION_DELETE_OPERATOR, username));
    if (const auto response = client.receive_msg(); response["status"] == MsgStatus::FAILED) {
        std::cout << "Reset password failed. The operator you entered does not exist." << std::endl;
        return;
    }

    client.send_msg(std::format(ACTION_ADD_OPERATOR, username, new_password));
    if (const auto response = client.receive_msg(); response["status"] == MsgStatus::SUCCESS)
        std::cout << "Password reset successfully." << std::endl;

    else {
        std::cout << "Failed to reset password as an unknown error happened." << std::endl;
        logger.warning(std::format("Unknown error happened while resetting operator's password: {}", response["message"]));
    }
}
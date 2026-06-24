#include "Users.h"

#include "../logger/Logger.h"
#include "../protocol/colors.h"
#include "../protocol/protocol.h"

#include <fstream>
#include <iostream>

static auto logger = Logger(__FILE__);

void SuperOperator::create_operator() const
{
    std::string username, password;
    std::cout << "Please enter new operator's username and password: " << std::endl;
    std::cin >> username >> password;

    client.send_msg(std::format(Action::CREATE_OPERATOR, username, password));
    const auto response = client.receive_msg();

    if (response["status"] == MsgStatus::SUCCESS)
        std::cout << OutputType::SUCCESS << "Operator added successfully." << OutputType::RESET << std::endl;
    else if (response["message"] == ErrorMsg::USER_EXISTS)
        std::cout << OutputType::WARNING << "Failed to add operator as username already exist. Please change one and try again." << OutputType::RESET << std::endl;
}

void SuperOperator::delete_operator() const
{
    std::string username;
    std::cout << "Please enter the username of the operator to delete: " << std::endl;
    std::cin >> username;

    client.send_msg(std::format(Action::DELETE_OPERATOR, username));
    const auto response = client.receive_msg();

    if (response["status"] == MsgStatus::SUCCESS)
        std::cout << OutputType::SUCCESS << "Operator deleted successfully." << OutputType::RESET << std::endl;
    else if (response["message"] == ErrorMsg::USER_NOT_FOUND)
        std::cout << OutputType::WARNING << "Failed to delete operator as the username does not exist." << OutputType::RESET << std::endl;
}

void SuperOperator::reset_operator_password() const
{
    std::string username, new_password;
    std::cout << "Enter the operator's username and new password: " << std::endl;
    std::cin >> username >> new_password;

    client.send_msg(std::format(Action::DELETE_OPERATOR, username));
    if (const auto response = client.receive_msg(); response["status"] == MsgStatus::FAILED) {
        std::cout << OutputType::ERROR << "Reset password failed. The operator you entered does not exist." << OutputType::RESET << std::endl;
        return;
    }

    client.send_msg(std::format(Action::CREATE_OPERATOR, username, new_password));
    if (const auto response = client.receive_msg(); response["status"] == MsgStatus::SUCCESS)
        std::cout << OutputType::SUCCESS << "Password reset successfully." << OutputType::RESET << std::endl;

    else {
        std::cout << OutputType::CRITICAL << "Failed to reset password as an unknown error happened." << OutputType::RESET << std::endl;
        logger.warning(std::format("Unknown error happened while resetting operator's password: {}", response["message"]));
    }
}

void SuperOperator::export_system_logs() const
{
    std::string filename;
    std::cout << "Please enter the filename to save system logs: " << std::endl;
    std::cin >> filename;

    std::ofstream out_file(filename + ".log");
    if (!out_file.is_open()) {
        std::cout << OutputType::ERROR << "Failed to create file. Please check the path or permissions." << OutputType::RESET << std::endl;
        return;
    }

    client.send_msg(Action::EXPORT_SERVER_LOGS.data());
    const auto start_msg = client.receive_msg();

    for (auto data_msg = client.receive_msg(); std::string(data_msg["message"]) != "END"; data_msg = client.receive_msg())
        out_file << std::string(data_msg["content"]) << "\n";

    out_file.close();
    std::cout << OutputType::SUCCESS << std::format("Successfully saved {} log lines to {}.", std::stoi(std::string(start_msg["length"])), filename) << OutputType::RESET << std::endl;
}
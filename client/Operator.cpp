#include "../stream/protocol.h"
#include "Users.h"

#include <iostream>

void Operator::add_student() const
{
    std::string real_name, gender, student_id, department;
    std::cout << "Please enter the info of student (RealName, Gender, StudentID, Department):" << std::endl;
    std::cin >> real_name >> gender >> student_id >> department;

    client.send_msg(std::format(ACTION_ADD_STUDENT, real_name, gender, student_id, department));

    if (const auto response = client.receive_msg(); response["status"] == MsgStatus::SUCCESS)
        std::cout << OutputType::SUCCESS << "Student registered successfully." << OutputType::RESET << std::endl;
    else if (response["message"] == ErrorMsg::USERINFO_EXISTS)
        std::cout << OutputType::ERROR << "Student register Failed: StudentID already exists." << OutputType::RESET << std::endl;
}

void Operator::recharge() const
{
    std::string card_number, amount_string;
    std::cout << "Enter the card number and recharge amount: " << std::endl;
    std::cin >> card_number >> amount_string;

    try {
        double amount = std::stod(amount_string);
        client.send_msg(std::format(ACTION_RECHARGE, card_number, std::format("{:.2f}", amount)));
    } catch (const std::exception&) {
        std::cout << "Invalid recharge amount format. You should enter a valid float." << std::endl;
        return;
    }

    if (const auto response = client.receive_msg(); response["status"] == MsgStatus::SUCCESS)
        std::cout << OutputType::SUCCESS << "Recharge successful." << OutputType::RESET << std::endl;
    else if (response["message"] == ErrorMsg::CARD_NOT_FOUND)
        std::cout << OutputType::ERROR << "Failed to recharge. The card number does not exist." << OutputType::RESET << std::endl;
    else if (response["message"] == ErrorMsg::ACCOUNT_ABNORMAL)
        std::cout << OutputType::ERROR << "Failed to recharge. The account is frozen or deleted." << OutputType::RESET << std::endl;
}

void Operator::manage_accounts() const
{
    std::string username, status_choice;

    std::cout << "Enter the username of the account and the index of action you want to perform: " << std::endl;
    std::cout << "1, freeze account. 2, delete account. 3, restore account." << std::endl;
    std::cin >> username >> status_choice;

    if (status_choice == "1")
        client.send_msg(std::format(ACTION_UPDATE_STATUS, username, UserStatus::FROZEN));
    else if (status_choice == "2")
        client.send_msg(std::format(ACTION_UPDATE_STATUS, username, UserStatus::DELETED));
    else if (status_choice == "3")
        client.send_msg(std::format(ACTION_UPDATE_STATUS, username, UserStatus::NORMAL));
    else {
        std::cout << OutputType::ERROR << "Invalid choice. Please enter number between 1-3." << OutputType::RESET << std::endl;
        return;
    }

    if (const auto response = client.receive_msg(); response["status"] == MsgStatus::SUCCESS)
        std::cout << OutputType::SUCCESS << "Account status updated successfully." << OutputType::RESET << std::endl;
    else if (response["message"] == ErrorMsg::USER_NOT_FOUND)
        std::cout << OutputType::ERROR << "Failed to update states. Username does not exist." << OutputType::RESET << std::endl;
}
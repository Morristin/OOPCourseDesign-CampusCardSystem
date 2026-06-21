#include "../stream/protocol.h"
#include "Users.h"

#include <iostream>

void Student::consume() const
{
    std::string merchant, amount_string;
    std::cout << "Please enter the merchant name and consumption amount provided by merchant: " << std::endl;
    std::cin >> merchant >> amount_string;

    try {
        double amount = std::stod(amount_string);
        client.send_msg(std::format(ACTION_CONSUME, user_information.card_number, std::format("{:.2f}", amount), merchant));
    } catch (const std::exception&) {
        std::cout << "Invalid amount format. You should enter a valid float." << std::endl;
        return;
    }

    if (const auto response = client.receive_msg(); response["status"] == MsgStatus::SUCCESS) {
        std::cout << "Consumption performed successful." << std::endl;
    } else if (response["message"] == ErrorMsg::BALANCE_INSUFFICIENT) {
        std::cout << "Consumption Failed. You do not have enough balance left." << std::endl;
    } else if (response["message"] == ErrorMsg::ACCOUNT_ABNORMAL) {
        std::cout << "Consumption Failed. Your account is frozen or deleted. Please contact operator." << std::endl;
    }
}
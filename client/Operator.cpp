#include "../stream/protocol.h"
#include "Users.h"

#include <iostream>

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
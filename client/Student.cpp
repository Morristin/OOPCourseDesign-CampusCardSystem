#include "Users.h"

#include "../protocol/colors.h"
#include "../protocol/protocol.h"

#include <iostream>

void Student::consume() const
{
    std::string merchant, amount_string;
    std::cout << "Please enter the merchant name and consumption amount provided by merchant: " << std::endl;
    std::cin >> merchant >> amount_string;

    try {
        double amount = std::stod(amount_string);
        client.send_msg(std::format(Action::CONSUME, user_information.card_number, std::format("{:.2f}", amount), merchant));
    } catch (const std::exception&) {
        std::cout << "Invalid amount format. You should enter a valid float." << std::endl;
        return;
    }

    if (const auto response = client.receive_msg(); response["status"] == MsgStatus::SUCCESS)
        std::cout << OutputType::SUCCESS << "Consumption performed successful." << OutputType::RESET << std::endl;
    else if (response["message"] == ErrorMsg::BALANCE_INSUFFICIENT)
        std::cout << OutputType::ERROR << "Consumption Failed. You do not have enough balance left." << OutputType::RESET << std::endl;
    else if (response["message"] == ErrorMsg::ACCOUNT_ABNORMAL)
        std::cout << OutputType::ERROR << "Consumption Failed. Your account is frozen or deleted. Please contact operator." << OutputType::RESET << std::endl;
}

void Student::set_consumption_limit() const
{
    std::string daily_str, single_str;
    std::cout << "Please enter Daily Limit and Single Transaction Limit (use '0' for no limit): " << std::endl;
    std::cin >> daily_str >> single_str;

    client.send_msg(std::format(Action::SET_CONSUMPTION_LIMIT, daily_str, single_str));

    if (const auto response = client.receive_msg(); response["status"] == MsgStatus::SUCCESS)
        std::cout << OutputType::SUCCESS << "Updated consumption limit successfully." << OutputType::RESET << std::endl;
    else
        std::cout << OutputType::ERROR << "Failed to update consumption limit: " << response["message"] << OutputType::RESET << std::endl;
}

void Student::query_own_records() const
{
    client.send_msg(Action::QUERY_OWN_TRANSACTION.data());

    auto start_msg = client.receive_msg();
    if (start_msg["status"] == MsgStatus::FAILED && start_msg["message"] == ErrorMsg::TRANSACTION_NOT_FOUND) {
        std::cout << OutputType::WARNING << "Cannot find any records on your card." << OutputType::RESET << std::endl;
        return;
    }

    std::cout << OutputType::SUCCESS << std::format("Successfully found {} records:", std::stoi(start_msg["length"])) << OutputType::RESET << std::endl;
    std::cout << OutputType::THEME << "                 Time  |  Amount  |  Balance  | OP / Merchant " << OutputType::RESET << std::endl;

    for (auto data_msg = client.receive_msg(); data_msg["message"] != "END"; data_msg = client.receive_msg())
        std::cout << std::format("  {}  |  {:>6.2f}  |  {:>7.2f}  | {}", data_msg["time"], std::stod(data_msg["amount"]), std::stod(data_msg["balance"]), data_msg["operator"]) << std::endl;
}
#include "../logger/Logger.h"
#include "Client.h"
#include "Users.h"

#include <iostream>

static auto logger = Logger(__FILE__);

std::unique_ptr<User> get_user(Client& client, const UserInformation& user_information)
{
    if (user_information.permission == 4)
        return std::make_unique<Student>(client, user_information);
    else if (user_information.permission == 6)
        return std::make_unique<Operator>(client, user_information);
    else if (user_information.permission == 7)
        return std::make_unique<SuperOperator>(client, user_information);
    else
        throw std::runtime_error("Unknown permission level");
}

int main()
{
    std::cout << OutputType::THEME << "Welcome to Campus Card Management System." << OutputType::RESET << std::endl;

    try {
        auto client = Client();
        client.start();
        const auto user_information = UserInformation(client.login());
        get_user(client, user_information)->show_dashboard();
    } catch (const std::exception& err) {
        logger.critical(std::format("Connection Lost or System Error: {}", err.what()));
    }
}
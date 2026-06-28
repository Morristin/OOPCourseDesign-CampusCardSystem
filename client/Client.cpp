#include "Client.h"

#include "../logger/Logger.h"
#include "../protocol/colors.h"
#include "../protocol/protocol.h"

#include <arpa/inet.h>
#include <iostream>

static auto logger = Logger(__FILE__);

Client::Client(const std::string& ip, int port) : Client()
{
    this->address = ip;
    this->port    = htons(port);
}

void Client::start() const
{
    sockaddr_in server_addr { };
    server_addr.sin_family = SIN_FAMILY;
    server_addr.sin_port   = port;

    inet_pton(SIN_FAMILY, address.c_str(), &server_addr.sin_addr);

    if (const auto status = connect(client, reinterpret_cast<struct sockaddr*>(&server_addr), sizeof(server_addr));
        status < 0) {
        logger.critical("Connection failed.");
        exit(1);
    }

    logger.info(std::format("Client started and connected to server: {}:{}", address, port));
}

Parser Client::receive_msg() const
{
    const auto message = stream.receive_msg();

    if (message["status"] == MsgStatus::FAILED && message["message"] == ErrorMsg::UNKNOWN_ACTION)
        logger.critical("Server cannot recognize the action. Please check your client version or contact operator.");
    else if (message["status"] == MsgStatus::FAILED && message["message"] == ErrorMsg::PERMISSION_DENIED)
        logger.error("You do not have the permission to perform this action. Please check your client version or contact operator.");

    return message;
}

Parser Client::login() const
{
    while (true) {
        std::string username, password;
        std::cout << "Enter your username and password here: " << std::endl;
        std::cin >> username >> password;

        send_msg(std::format(Action::LOGIN, username, password));
        const auto message = receive_msg();

        if (message["status"] == MsgStatus::SUCCESS && std::stoi(message["user_status"]) == UserStatus::NORMAL) {
            std::cout << "Welcome, " << username << "!" << std::endl;
            return message;
        }

        if (message["status"] == MsgStatus::FAILED && message["message"] == ErrorMsg::USER_NOT_FOUND)
            std::cout << OutputType::ERROR << "You have not register yet. Check your name or contact operator." << OutputType::RESET << std::endl;
        else if (message["status"] == MsgStatus::FAILED && message["message"] == ErrorMsg::PASSWORD_WRONG)
            std::cout << OutputType::ERROR << "Your password is not correct. Please try again." << OutputType::RESET << std::endl;

        if (message["status"] == MsgStatus::SUCCESS && std::stoi(message["user_status"]) == UserStatus::DELETED)
            std::cout << OutputType::ERROR << "Your account has been deleted. Contact operator for help." << OutputType::RESET << std::endl;
        else if (message["status"] == MsgStatus::SUCCESS && std::stoi(message["user_status"]) == UserStatus::FROZEN)
            std::cout << OutputType::ERROR << "Your account has been frozen. Please contact operator." << OutputType::RESET << std::endl;
        else if (message["status"] == MsgStatus::SUCCESS && std::stoi(message["user_status"]) == UserStatus::OVERDRAWN) {
            std::cout << OutputType::WARNING << "Your account is overdrawn. Please recharge immediately." << OutputType::RESET << std::endl;
            std::cout << "You can still view your records, but consumption is banned as your status is abnormal." << std::endl;
            return message;
        }
    }
}
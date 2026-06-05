#include "Client.h"

#include "../logger/Logger.h"
#include "../stream/protocol.h"

#include <arpa/inet.h>

static constexpr auto CONNECT_FAILED = 32;
static auto logger = Logger(__FILE__);

Client::Client(const std::string& ip, int port) : Client()
{
    this->address = ip;
    this->port = htons(port);
}

[[noreturn]] void Client::start() const
{
    sockaddr_in server_addr { };
    server_addr.sin_family = SIN_FAMILY;
    server_addr.sin_port = port;

    inet_pton(SIN_FAMILY, address.c_str(), &server_addr.sin_addr);

    if (const auto status = connect(client, reinterpret_cast<struct sockaddr*>(&server_addr), sizeof(server_addr));
        status < 0) {
        logger.critical("Connection failed.");
        exit(CONNECT_FAILED);
    }

    std::cout << "Welcome to Campus Card Management System." << std::endl;
    while (!login()) { }
    while (true) { }
}

int Client::login() const
{
    std::string username, password;
    std::cout << "Enter your username and password here: " << std::endl;
    std::cin >> username >> password;

    if (username.find(',') == std::string::npos && username.find(':') == std::string::npos) {
        send_msg(std::format(ACTION_LOGIN, username, password));
    } else {
        std::cout << "You can not include special character in username" << std::endl;
        return false;
    }

    const auto message = receive_msg();
    if (message["status"] == MsgStatus::SUCCESS && std::stoi(message["user_status"]) == UserStatus::NORMAL) {
        std::cout << "Welcome, " << username << "!" << std::endl;
        return std::stoi(message["permission"]);
    }

    if (message["status"] == MsgStatus::FAILED && message["message"] == ErrorMsg::USER_NOT_FOUND)
        std::cout << "You have not register yet. Check your name or contact operator." << std::endl;
    else if (message["status"] == MsgStatus::FAILED && message["message"] == ErrorMsg::WRONG_PASSWORD)
        std::cout << "Your password is not correct. Please try again." << std::endl;

    else if (std::stoi(message["user_status"]) == UserStatus::DELETED)
        std::cout << "Your account has been deleted. Contact operator for help." << std::endl;
    else if (std::stoi(message["user_status"]) == UserStatus::FROZEN)
        std::cout << "Your account has been frozen. Please contact operator." << std::endl;

    return false;
}
#include "Server.h"

#include "../logger/Logger.h"
#include "../stream/protocol.h"
#include "Database.h"

#include <netinet/in.h>

static constexpr auto BIND_ADDRESS_FAILED = 21;
static constexpr auto LISTEN_FAILED = 22;
static auto logger = Logger(__FILE__);

[[noreturn]] void Server::start()
{
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &ALLOW_PORT_REUSE, sizeof(ALLOW_PORT_REUSE));

    sockaddr_in server_addr { };
    server_addr.sin_family = SIN_FAMILY;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(DEFAULT_PORT);

    if (const auto status = bind(server, reinterpret_cast<struct sockaddr*>(&server_addr), sizeof(server_addr));
        status < 0) {
        logger.critical("Can not bind address to server.");
        exit(BIND_ADDRESS_FAILED);
    }

    if (const auto status = listen(server, BACKLOG); status < 0) {
        logger.critical("Can not start listen.");
        exit(LISTEN_FAILED);
    }

    logger.info("Server started.");
    std::cout << "Server is started. Listen on port " << server_addr.sin_port << "." << std::endl;

    while (true) {
        sockaddr_in client_addr { };
        socklen_t client_addr_len = sizeof(client_addr);

        const int client = accept(server, reinterpret_cast<struct sockaddr*>(&client_addr), &client_addr_len);
        if (client < 0) {
            logger.error("Failed to accept client connection.");
            std::cout << "Failed to accept client connection." << std::endl;
            continue;
        }

        const auto stream = Stream(client);

        try {
            while (true) {
                auto msg = stream.receive_msg();

                if (std::string action = msg["action"]; action == "login")
                    handle_login(stream, msg);
                else if (action == "add_operator")
                    handle_add_operator(stream, msg);
            }
        } catch (const std::exception& err) {
            logger.warning(std::format("Client handling error: {}", err.what()));
            close(client);
        }
    }
}

void Server::handle_login(const Stream& stream, const Parser& message)
{
    const auto username = message["username"], password = message["password"];
    try {
        const auto login_user_status = database.check_identity(username, password);
        stream.send_msg(login_user_status.message());
        logger.info(std::format("User {} successfully logged in.", username));
    } catch (const DatabaseException& err) {
        if (err.what() == ErrorMsg::DATABASE_FIND_USER_FAILED)
            logger.warning(std::format("Can not find user: {}. Check database and SQL script.", username));
        stream.send_msg(std::format(STATUS_WITH_MSG, MsgStatus::FAILED, err.what()));
    }
}

void Server::handle_add_operator(const Stream& stream, const Parser& message)
{
    const std::string username = message["username"];
    const std::string password = message["password"];

    try {
        database.add_operator(username, password);
        stream.send_msg(std::format(STATUS_WITH_MSG, MsgStatus::SUCCESS, ""));
    } catch (const DatabaseException& err) {
        if (err.what() == ErrorMsg::USER_ALREADY_EXISTS)
            logger.warning(std::format("Try to add operator {} failed as username already exist.", username));
        stream.send_msg(std::format(STATUS_WITH_MSG, MsgStatus::FAILED, err.what()));
    }
}
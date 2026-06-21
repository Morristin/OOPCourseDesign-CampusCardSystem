#include "Server.h"

#include "../logger/Logger.h"
#include "../stream/protocol.h"
#include "Database.h"

#include <netinet/in.h>
#include <thread>

static constexpr auto BIND_ADDRESS_FAILED = 21;
static constexpr auto LISTEN_FAILED = 22;
static auto logger = Logger(__FILE__);

Server::Server()
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
}

[[noreturn]] void Server::start()
{
    while (true) {
        sockaddr_in client_addr { };
        socklen_t client_addr_len = sizeof(client_addr);
        const int client = accept(server, reinterpret_cast<struct sockaddr*>(&client_addr), &client_addr_len);

        std::thread([client, this]() {
            auto client_logger = Logger(std::format("Client {}", client));

            auto stream = Stream(client);
            Session session { stream, Permission::DEFAULT, Parser("") };

            try {
                while (true) {
                    session.message = stream.receive_msg();
                    const std::string action = session.message["action"];

                    if (auto route = routes.find(action); route == routes.end()) {
                        stream.send_msg(std::format(STATUS_WITH_MSG, MsgStatus::FAILED, ErrorMsg::UNKNOWN_ACTION));
                        client_logger.error(std::format("Received an unknown action: {}", action));
                    }

                    else if (session.permission >= route->second.permission_requirement)
                        route->second.handler(session);

                    else {
                        stream.send_msg(std::format(STATUS_WITH_MSG, MsgStatus::FAILED, ErrorMsg::PERMISSION_DENIED));
                        client_logger.warning(std::format("Permission denied: {} require {}. Current permission: {}.", action, route->second.permission_requirement, session.permission));
                    }
                }
            } catch (const std::exception& err) {
                client_logger.warning(std::format("Client handling error: {}", err.what()));
                close(client);
            }
        }).detach();
    }
}

void Server::handle_login(Session& session)
{
    const auto username = session.message["username"];
    const auto password = session.message["password"];

    try {
        const auto login_user_status = database.check_identity(username, password);
        session.permission = std::stoi(login_user_status.permission.data());

        session.stream.send_msg(login_user_status.message());
        logger.info(std::format("User {} successfully logged in.", username));
    } catch (const DatabaseException& err) {
        if (err.what() == ErrorMsg::DATABASE_FIND_USER_FAILED)
            logger.warning(std::format("Can not find user: {}. Check database and SQL script.", username));
        session.stream.send_msg(std::format(STATUS_WITH_MSG, MsgStatus::FAILED, err.what()));
    }
}

void Server::handle_add_operator(const Session& session)
{
    const auto username = session.message["username"];
    const auto password = session.message["password"];

    try {
        database.add_operator(username, password);
        session.stream.send_msg(std::format(STATUS_WITH_MSG, MsgStatus::SUCCESS, ""));
    } catch (const DatabaseException& err) {
        if (err.what() == ErrorMsg::USER_ALREADY_EXISTS)
            logger.warning(std::format("Try to add operator {} failed as username already exist.", username));
        session.stream.send_msg(std::format(STATUS_WITH_MSG, MsgStatus::FAILED, err.what()));
    }
}

void Server::handle_del_operator(const Session& session)
{
    const auto username = session.message["username"];

    try {
        database.del_operator(username);
        session.stream.send_msg(std::format(STATUS_WITH_MSG, MsgStatus::SUCCESS, ""));
    } catch (const DatabaseException& err) {
        if (err.what() == ErrorMsg::USER_NOT_FOUND)
            logger.warning(std::format("Try to delete operator {} failed as user not found.", username));
        session.stream.send_msg(std::format(STATUS_WITH_MSG, MsgStatus::FAILED, err.what()));
    }
}
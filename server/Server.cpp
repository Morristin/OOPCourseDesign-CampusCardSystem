#include "Server.h"

#include "../logger/Logger.h"
#include "../stream/protocol.h"
#include "Database.h"

#include <netinet/in.h>
#include <thread>

static constexpr auto BIND_ADDRESS_FAILED = 21;
static constexpr auto LISTEN_FAILED = 22;
static auto logger = Logger(__FILE__);

/**
 * Execute the func given and generate simple response.
 *
 * @param session The session is used to send response message with its stream.
 * @param func It's recommended to use lambda function directly.
 * @return If any exception is caught, return err.what(). Else return blank string.
 *
 * For example, @code std::string err = execute_with_response(session, [&]() { func(); });
 */
template <typename Func>
std::string Server::execute_and_response(const Session& session, Func&& func)
{
    try {
        func();
        session.stream.send_msg(std::format(STATUS_WITH_MSG, MsgStatus::SUCCESS, ""));
        return "";
    } catch (const DatabaseException& err) {
        session.stream.send_msg(std::format(STATUS_WITH_MSG, MsgStatus::FAILED, err.what()));
        return err.what();
    }
}

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
            auto stream = Stream(client);
            auto client_logger = Logger(std::format("Client {}", client));
            Session session { stream, client_logger, "", Permission::DEFAULT, Parser("") };

            try {
                while (true) {
                    session.message = stream.receive_msg();
                    const std::string action = session.message["action"];

                    if (auto route = routes.find(action); route == routes.end()) {
                        stream.send_msg(std::format(STATUS_WITH_MSG, MsgStatus::FAILED, ErrorMsg::UNKNOWN_ACTION));
                        session.logger.error(std::format("Received an unknown action: {}", action));
                    }

                    else if (session.permission >= route->second.permission_requirement)
                        route->second.handler(session);

                    else {
                        stream.send_msg(std::format(STATUS_WITH_MSG, MsgStatus::FAILED, ErrorMsg::PERMISSION_DENIED));
                        session.logger.warning(std::format("Permission denied: {} require {}. Current permission: {}.", action, route->second.permission_requirement, session.permission));
                    }
                }
            } catch (const std::exception& err) {
                session.logger.warning(std::format("Client handling error: {}", err.what()));
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
        session.username = login_user_status.username;
        session.permission = std::stoi(login_user_status.permission.data());

        session.stream.send_msg(login_user_status.message());
        session.logger.info(std::format("User {} successfully logged in.", username));
    } catch (const DatabaseException& err) {
        session.stream.send_msg(std::format(STATUS_WITH_MSG, MsgStatus::FAILED, err.what()));
    }
}

void Server::handle_add_operator(const Session& session)
{
    const auto username = session.message["username"];
    const auto password = session.message["password"];
    const auto err = execute_and_response(session, [&]() { database.create_account(username, password, Permission::OPERATOR); });

    if (err == ErrorMsg::USER_ALREADY_EXISTS)
        session.logger.warning(std::format("Try to add operator {} failed as username already exist.", username));
}

void Server::handle_del_operator(const Session& session)
{
    const auto username = session.message["username"];
    const auto err = execute_and_response(session, [&]() { database.del_operator(username); });

    if (err == ErrorMsg::USER_NOT_FOUND)
        session.logger.warning(std::format("Try to delete operator {} failed as user not found.", username));
}

void Server::handle_add_student(const Session& session)
{
    const std::string real_name = session.message["real_name"];
    const std::string gender = session.message["gender"];
    const std::string student_id = session.message["student_id"];
    const std::string department = session.message["department"];

    const auto err = execute_and_response(session, [&]() { database.register_student(real_name, gender, student_id, department); });
}

void Server::handle_del_student(const Session& session)
{
    const std::string student_id = session.message["student_id"];
    const auto err = execute_and_response(session, [&]() { database.delete_student(student_id); });
}

void Server::handle_update_student(const Session& session)
{
    const std::string student_id = session.message["student_id"];
    const std::string real_name = session.message["real_name"];
    const std::string gender = session.message["gender"];
    const std::string department = session.message["department"];

    const auto err = execute_and_response(session, [&]() { database.update_student(student_id, real_name, gender, department); });
}

void Server::handle_recharge(const Session& session)
{
    const std::string card_number = session.message["card_number"];
    const double amount = std::stod(session.message["amount"]);

    const auto err = execute_and_response(session, [&]() { database.recharge_card(card_number, amount, session.username); });
}

void Server::handle_update_status(const Session& session)
{
    const std::string username = session.message["username"];
    const int new_status = std::stoi(session.message["status"]);

    const auto err = execute_and_response(session, [&]() { database.update_account_status(username, new_status); });
}

void Server::handle_consume(const Session& session)
{
    const std::string card_number = session.message["card_number"];
    const double amount = std::stod(session.message["amount"]);
    const std::string merchant = session.message["merchant"];

    const auto err = execute_and_response(session, [&]() { database.consume_card(card_number, amount, merchant); });
}

void Server::handle_query_transactions(const Session& session)
{
    const std::string card_number = session.message["card_number"];

    try {
        const auto records = database.query_transactions(card_number);
        session.stream.send_msg(std::format(LONG_MSG_START, MsgStatus::SUCCESS, records.size()));
        for (const auto& rec : records)
            session.stream.send_msg(rec);
        session.stream.send_msg(std::format(LONG_MSG_END, MsgStatus::SUCCESS, records.size()));
    } catch (const DatabaseException& err) {
        session.stream.send_msg(std::format(STATUS_WITH_MSG, MsgStatus::FAILED, err.what()));
    }
}
#include "Server.h"

#include "../logger/Logger.h"
#include "../protocol/colors.h"
#include "../protocol/protocol.h"
#include "Database.h"

#include <csignal>
#include <iostream>
#include <netinet/in.h>
#include <thread>

static auto logger = Logger(__FILE__);

/**
 * Execute the func given and generate simple response.
 *
 * Usage: @code std::string err = execute_with_response(session, [&]() { func(); }); @endcode
 *
 * @param session Used to provide stream to send response message with.
 * @param func It's recommended to use lambda function.
 * @return If any exception is caught, return err.what(). Else return blank string.
 */
template <typename Func>
std::string execute_and_response(const Session& session, Func&& func)
{
    try {
        func();
        return "";
    } catch (const DatabaseException& err) {
        session.stream.send_msg(std::format(STATUS_WITH_MSG, MsgStatus::FAILED, err.what()));
        return err.what();
    }
}

template <typename Func>
std::string execute_and_response_long(const Session& session, Func&& func)
{
    try {
        const auto records = func();
        session.stream.send_msg(std::format(LONG_MSG_START, MsgStatus::SUCCESS, records.size()));
        for (const auto& rec : records)
            session.stream.send_msg(rec);
        session.stream.send_msg(std::format(LONG_MSG_END, MsgStatus::SUCCESS, records.size()));
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
    server_addr.sin_family      = SIN_FAMILY;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port        = htons(DEFAULT_PORT);

    std::signal(SIGPIPE, SIG_IGN); // To prevent server exit when pipe broken.

    if (const auto status = bind(server, reinterpret_cast<struct sockaddr*>(&server_addr), sizeof(server_addr)); status < 0)
        logger.critical("Can not bind address to server.");
    if (const auto status = listen(server, BACKLOG); status < 0)
        logger.critical("Can not start listen.");

    logger.info("Server started.");
    std::cout << OutputType::THEME << std::format("Server is started. Listen on port {}.", server_addr.sin_port) << OutputType::RESET << std::endl;
}

[[noreturn]] void Server::start()
{
    while (true) {
        sockaddr_in client_addr { };
        socklen_t client_addr_len = sizeof(client_addr);
        const int client          = accept(server, reinterpret_cast<struct sockaddr*>(&client_addr), &client_addr_len);

        std::thread([client, this]() {
            auto stream        = Stream(client);
            auto client_logger = Logger(std::format("Client {}", client));
            Session session { stream, client_logger, "", Permission::DEFAULT, Parser("") };

            session.logger.info(std::format("Successfully connected client {}", client));
            std::cout << OutputType::THEME << std::format("Client {} connected.", client) << OutputType::RESET << std::endl;

            try {
                while (true) {
                    session.message = stream.receive_msg();
                    if (session.message["status"] == MsgStatus::FAILED && session.message["message"] == ErrorMsg::NETWORK_ERROR) {
                        session.logger.info("Client disconnected.");
                        std::cout << OutputType::SUCCESS << std::format("Client {} disconnected.", client) << OutputType::RESET << std::endl;
                        break;
                    }

                    const std::string action = session.message["action"];

                    for (const auto& background_task : background_tasks)
                        background_task();

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
                std::cout << OutputType::WARNING << std::format("Client {} disconnected as an error happened.", client) << OutputType::RESET << std::endl;
            }

            close(client);
        }).detach();
    }
}

void Server::handle_login(Session& session)
{
    const std::string username = session.message["username"];
    execute_and_response(session, [&] {
        const auto user_info = Parser(database.query_account(username));

        // If password wrong, throw PASSWORD_WRONG for execute_and_response() to handle.
        if (const std::string input_password = session.message["password"]; user_info["password"] != input_password)
            throw DatabaseException(ErrorMsg::PASSWORD_WRONG);

        // Update the username and permission in the session.
        session.username   = username;
        session.permission = std::stoi(std::string(user_info["permission"]));

        // Generate and send message to client.
        session.stream.send_msg(std::format(LOGIN_RESULT, MsgStatus::SUCCESS, username, user_info["permission"], user_info["status"], user_info["card_number"]));
    });
}

void Server::handle_update_system_settings(const Session& session)
{
    const std::string key   = session.message["key"];
    const std::string value = session.message["value"];

    execute_and_response(session, [&] {database.update_system_setting(key, value);
        session.stream.send_msg(std::format(STATUS_WITH_MSG, MsgStatus::SUCCESS, "")); });
}

void Server::handle_export_server_logs(const Session& session)
{
    const auto logs = Logger::to_vector();
    std::vector<std::string> formatted_logs;
    formatted_logs.reserve(logs.size());
    for (const auto& line : logs)
        formatted_logs.emplace_back(std::format(LOG_LINE, line));
    execute_and_response_long(session, [&] { return formatted_logs; });
}

void Server::handle_create_operator(const Session& session)
{
    const auto username = session.message["username"];
    const auto password = session.message["password"];
    const auto err      = execute_and_response(session, [&] { database.create_account(username, password, Permission::OPERATOR);
        session.stream.send_msg(std::format(STATUS_WITH_MSG, MsgStatus::SUCCESS, "")); });

    if (err == ErrorMsg::USER_EXISTS)
        session.logger.warning(std::format("Try to add operator {} failed as username already exist.", username));
}

void Server::handle_delete_operator(const Session& session)
{
    const auto username = session.message["username"];
    const auto err      = execute_and_response(session, [&] { database.delete_operator(username);
        session.stream.send_msg(std::format(STATUS_WITH_MSG, MsgStatus::SUCCESS, "")); });

    if (err == ErrorMsg::USER_NOT_FOUND)
        session.logger.warning(std::format("Try to delete operator {} failed as user not found.", username));
}

void Server::handle_create_student(const Session& session)
{
    const std::string real_name  = session.message["real_name"];
    const std::string gender     = session.message["gender"];
    const std::string student_id = session.message["student_id"];
    const std::string department = session.message["department"];

    const auto err = execute_and_response(session, [&] { database.create_student(real_name, gender, student_id, department);
        session.stream.send_msg(std::format(STATUS_WITH_MSG, MsgStatus::SUCCESS, "")); });
}

void Server::handle_delete_student(const Session& session)
{
    const std::string student_id = session.message["student_id"];
    const auto err               = execute_and_response(session, [&] { database.delete_student(student_id);
        session.stream.send_msg(std::format(STATUS_WITH_MSG, MsgStatus::SUCCESS, "")); });
}

void Server::handle_update_student_status(const Session& session)
{
    const std::string username = session.message["username"];
    const int new_status       = std::stoi(session.message["status"]);

    const auto err = execute_and_response(session, [&] { database.update_account_status(username, new_status);
        session.stream.send_msg(std::format(STATUS_WITH_MSG, MsgStatus::SUCCESS, "")); });
}

void Server::handle_update_student_userinfo(const Session& session)
{
    const std::string student_id = session.message["student_id"];
    const std::string real_name  = session.message["real_name"];
    const std::string gender     = session.message["gender"];
    const std::string department = session.message["department"];

    const auto err = execute_and_response(session, [&] { database.update_student(student_id, real_name, gender, department);
        session.stream.send_msg(std::format(STATUS_WITH_MSG, MsgStatus::SUCCESS, "")); });
}

void Server::handle_recharge(const Session& session)
{
    const std::string card_number = session.message["card_number"];
    const double amount           = std::stod(session.message["amount"]);

    const auto err = execute_and_response(session, [&] { database.recharge_card(card_number, amount, session.username);
        session.stream.send_msg(std::format(STATUS_WITH_MSG, MsgStatus::SUCCESS, "")); });
}

void Server::handle_consume(const Session& session)
{
    const std::string card_number = session.message["card_number"];
    const double amount           = std::stod(session.message["amount"]);
    const std::string merchant    = session.message["merchant"];

    const auto err = execute_and_response(session, [&] { database.consume_card(card_number, amount, merchant, false);
        session.stream.send_msg(std::format(STATUS_WITH_MSG, MsgStatus::SUCCESS, "")); });
}

void Server::handle_query_abnormal_accounts(const Session& session)
{
    execute_and_response_long(session, [&] { return database.query_abnormal_accounts(); });
}

void Server::handle_query_transactions(const Session& session)
{
    const std::string card_number = session.message["card_number"];
    execute_and_response_long(session, [&] { return database.query_transactions(card_number); });
}

void Server::handle_query_own_transactions(Session& session)
{
    // Get the real information of user using data stored in session.
    const Parser user_info(database.query_account(session.username));

    // Replace the message in session and reuse the Server::handle_query_transactions().
    session.message = Parser(std::format(Action::QUERY_TRANSACTION, user_info["card_number"]));
    handle_query_transactions(session);
}

void Server::handle_query_merchant(const Session& session)
{
    const std::string merchant = session.message["merchant"];
    execute_and_response_long(session, [&]() { return database.query_merchant_transactions(merchant); });
}

void Server::handle_export_transactions(const Session& session)
{
    execute_and_response_long(session, [&] { return database.export_transactions(); });
}

void Server::handle_generate_statistics(const Session& session)
{
    const std::string type = session.message["type"];
    execute_and_response_long(session, [&] { return database.generate_statistics(type); });
}
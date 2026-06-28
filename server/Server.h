#ifndef SERVER_H
#define SERVER_H

#include "../logger/Logger.h"
#include "../stream/Stream.h"
#include "Database.h"

#include <sys/socket.h>
#include <unistd.h>

struct Session {
    Stream& stream;
    Logger& logger;

    std::string username;
    int permission = Permission::DEFAULT;

    Parser message;
};

class Server {
private:
    // Server Configuration
    static constexpr auto SIN_FAMILY   = AF_INET;
    static constexpr auto SOCKET_TYPE  = SOCK_STREAM;
    static constexpr auto DEFAULT_PORT = 8080;

    static constexpr auto ALLOW_PORT_REUSE = 1;
    static constexpr auto BACKLOG          = 3;

    int server = socket(SIN_FAMILY, SOCKET_TYPE, 0);

    // Database Connector
    Database database = Database();

    // Route & Background Tasks
    struct Route {
        int permission_requirement;
        std::function<void(Session&)> handler;
    };

    std::unordered_map<std::string_view, Route> routes {
        { "login", { 0, [this](Session& s) { handle_login(s); } } },
        { "update_system_settings", { Permission::OPERATOR, [this](const Session& s) { handle_update_system_settings(s); } } },
        { "export_server_logs", { Permission::SUPEROPERATOR, [](const Session& s) { handle_export_server_logs(s); } } },

        { "create_operator", { Permission::SUPEROPERATOR, [this](const Session& s) { handle_create_operator(s); } } },
        { "delete_operator", { Permission::SUPEROPERATOR, [this](const Session& s) { handle_delete_operator(s); } } },

        { "create_student", { Permission::OPERATOR, [this](const Session& s) { handle_create_student(s); } } },
        { "delete_student", { Permission::OPERATOR, [this](const Session& s) { handle_delete_student(s); } } },
        { "update_student_status", { Permission::OPERATOR, [this](const Session& s) { handle_update_student_status(s); } } },
        { "update_student_userinfo", { Permission::OPERATOR, [this](const Session& s) { handle_update_student_userinfo(s); } } },

        { "recharge", { Permission::OPERATOR, [this](const Session& s) { handle_recharge(s); } } },
        { "consume", { Permission::STUDENT, [this](const Session& s) { handle_consume(s); } } },
        { "set_consumption_limit", { Permission::STUDENT, [this](const Session& s) { handle_set_consumption_limit(s); } } },

        { "query_abnormal_accounts", { Permission::OPERATOR, [this](const Session& s) { handle_query_abnormal_accounts(s); } } },
        { "query_transaction", { Permission::OPERATOR, [this](const Session& s) { handle_query_transactions(s); } } },
        { "query_own_transaction", { Permission::STUDENT, [this](Session& s) { handle_query_own_transactions(s); } } },
        { "query_merchant", { Permission::OPERATOR, [this](Session& s) { handle_query_merchant(s); } } },
        { "export_transaction", { Permission::OPERATOR, [this](const Session& s) { handle_export_transactions(s); } } },
        { "generate_statistics", { Permission::OPERATOR, [this](const Session& s) { handle_generate_statistics(s); } } }
    };

    std::vector<std::function<void()>> background_tasks = {
        [this] { database.check_and_deduct_fixed_fee(); }
    };

public:
    Server();
    ~Server() { close(server); }

    void run();

    void handle_login(Session& session);
    void handle_update_system_settings(const Session& session);
    static void handle_export_server_logs(const Session& session);

    void handle_create_operator(const Session& session);
    void handle_delete_operator(const Session& session);

    void handle_create_student(const Session& session);
    void handle_delete_student(const Session& session);
    void handle_update_student_status(const Session& session);
    void handle_update_student_userinfo(const Session& session);

    void handle_recharge(const Session& session);
    void handle_consume(const Session& session);
    void handle_set_consumption_limit(const Session& session);

    void handle_query_abnormal_accounts(const Session& session);
    void handle_query_transactions(const Session& session);
    void handle_query_own_transactions(Session& session);
    void handle_query_merchant(const Session& session);
    void handle_export_transactions(const Session& session);
    void handle_generate_statistics(const Session& session);
};

#endif

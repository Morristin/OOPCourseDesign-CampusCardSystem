#ifndef SERVER_H
#define SERVER_H

#include "../stream/Stream.h"
#include "Database.h"

#include <sys/socket.h>
#include <unistd.h>

struct Session {
    Stream& stream;
    std::string username;
    int permission = Permission::DEFAULT;
    Parser message;
};

class Server {
private:
    // Server Configuration
    static constexpr auto SIN_FAMILY = AF_INET;
    static constexpr auto SOCKET_TYPE = SOCK_STREAM;
    static constexpr auto DEFAULT_PORT = 8080;

    static constexpr auto ALLOW_PORT_REUSE = 1;
    static constexpr auto BACKLOG = 3;

    int server = socket(SIN_FAMILY, SOCKET_TYPE, 0);

    // Database Connector
    Database database = Database();

    // Route Struct & Route Map
    struct Route {
        int permission_requirement;
        std::function<void(Session&)> handler;
    };

    std::unordered_map<std::string_view, Route> routes {
        { "login", { 0, [this](Session& s) { handle_login(s); } } },
        { "add_operator", { Permission::SUPEROPERATOR, [this](const Session& s) { handle_add_operator(s); } } },
        { "del_operator", { Permission::SUPEROPERATOR, [this](const Session& s) { handle_del_operator(s); } } },
        { "add_student", { Permission::OPERATOR, [this](const Session& s) { handle_add_student(s); } } },
        { "recharge", { Permission::OPERATOR, [this](const Session& s) { handle_recharge(s); } } },
        { "consume", { Permission::STUDENT, [this](const Session& s) { handle_consume(s); } } }
    };

public:
    Server();
    ~Server() { close(server); }

    [[noreturn]] void start();
    void handle_login(Session& session);

    void handle_add_operator(const Session& session);
    void handle_del_operator(const Session& session);
    void handle_add_student(const Session& session);

    void handle_recharge(const Session& session);
    void handle_update_status(const Session& session);

    void handle_consume(const Session& session);
};

#endif

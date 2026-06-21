#ifndef SERVER_H
#define SERVER_H

#include "../stream/Stream.h"
#include "Database.h"

#include <sys/socket.h>
#include <unistd.h>

class Server {
private:
    static constexpr auto SIN_FAMILY = AF_INET;
    static constexpr auto SOCKET_TYPE = SOCK_STREAM;
    static constexpr auto DEFAULT_PORT = 8080;

    static constexpr auto ALLOW_PORT_REUSE = 1;
    static constexpr auto BACKLOG = 3;

    int server = socket(SIN_FAMILY, SOCKET_TYPE, 0);
    Database database = Database();

public:
    Server() = default;
    ~Server() { close(server); }

    [[noreturn]] void start();
    void handle_login(const Stream& stream, const Parser& message);

    void handle_add_operator(const Stream& stream, const Parser& message);
    void handle_del_operator(const Stream& stream, const Parser& message);
};

#endif

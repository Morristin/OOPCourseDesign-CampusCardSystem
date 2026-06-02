#ifndef SERVER_H
#define SERVER_H

#include <sys/socket.h>

class Server {
private:
    static constexpr auto SIN_FAMILY = AF_INET;
    static constexpr auto SOCKET_TYPE = SOCK_STREAM;
    static constexpr auto DEFAULT_PORT = htons(8080);
    static constexpr auto ALLOW_PORT_REUSE = 1;

    static constexpr auto BACKLOG = 3;

public:
    [[noreturn]] static void start();
};

#endif

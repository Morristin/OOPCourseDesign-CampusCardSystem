#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <sys/socket.h>
#include <unistd.h>

class Client {
private:
    static constexpr auto SIN_FAMILY = AF_INET;
    static constexpr auto SOCKET_TYPE = SOCK_STREAM;

    static constexpr auto SERVER_ADDR = "127.0.0.1";
    static constexpr auto SERVER_PORT = htons(8080);

    std::string address = SERVER_ADDR;
    int port = SERVER_PORT;
    int client = socket(SIN_FAMILY, SOCKET_TYPE, 0);

public:
    Client() = default;
    Client(const std::string& ip, int port);
    ~Client() { close(client); }

    void start() const;
};

#endif

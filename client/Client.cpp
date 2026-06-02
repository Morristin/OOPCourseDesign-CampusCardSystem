#include "Client.h"

#include "../logger/Logger.h"

#include <arpa/inet.h>

static constexpr auto BIND_ADDRESS_FAILED = 21;
static constexpr auto CONNECT_FAILED = 32;
auto logger = Logger(__FILE__);

Client::Client(const std::string& ip, int port) : Client()
{
    this->address = ip;
    this->port = htons(port);
}

void Client::start() const
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
}

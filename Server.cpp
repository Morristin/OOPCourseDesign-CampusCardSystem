#include "Server.h"
#include "logger/Logger.h"

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

static constexpr auto BIND_ADDRESS_FAILED = 21;
static constexpr auto LISTEN_FAILED = 22;
auto logger = Logger(__FILE__);

void Server::start()
{
    const int server = socket(SIN_FAMILY, SOCKET_TYPE, 0);
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &ALLOW_PORT_REUSE, sizeof(ALLOW_PORT_REUSE));

    sockaddr_in address { };
    address.sin_family = SIN_FAMILY;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = DEFAULT_PORT;

    // Bind address to server. Handle exceptions.
    if (const auto status = bind(server, reinterpret_cast<struct sockaddr*>(&address), sizeof(address)); status < 0) {
        logger.critical("Can not bind address to server.");
        close(server);
        exit(BIND_ADDRESS_FAILED);
    }

    // Let server start listen. Handle exceptions.
    if (const auto status = listen(server, BACKLOG); status < 0) {
        logger.critical("Can not start listen.");
        close(server);
        exit(LISTEN_FAILED);
    }
}
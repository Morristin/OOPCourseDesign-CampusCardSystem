#include "Server.h"
#include "../logger/Logger.h"
#include "../stream/protocol.h"

#include <netinet/in.h>

static constexpr auto BIND_ADDRESS_FAILED = 21;
static constexpr auto LISTEN_FAILED = 22;
static auto logger = Logger(__FILE__);

[[noreturn]] void Server::start() const
{
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &ALLOW_PORT_REUSE, sizeof(ALLOW_PORT_REUSE));

    sockaddr_in server_addr { };
    server_addr.sin_family = SIN_FAMILY;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(DEFAULT_PORT);

    // Bind address to server. Handle exceptions.
    if (const auto status = bind(server, reinterpret_cast<struct sockaddr*>(&server_addr), sizeof(server_addr));
        status < 0) {
        logger.critical("Can not bind address to server.");
        exit(BIND_ADDRESS_FAILED);
    }

    // Let server start listen. Handle exceptions.
    if (const auto status = listen(server, BACKLOG); status < 0) {
        logger.critical("Can not start listen.");
        exit(LISTEN_FAILED);
    }

    while (true) {
        sockaddr_in client_addr { };
        socklen_t client_addr_len = sizeof(client_addr);

        const int client = accept(server, reinterpret_cast<struct sockaddr*>(&client_addr), &client_addr_len);
        if (client < 0) {
            logger.error("Failed to accept client connection.");
            std::cout << "Failed to accept client connection." << std::endl;
            continue;
        }

        auto stream = Stream(client);
        try {
            handle_login(stream);
        } catch (const std::exception& err) {
            logger.warning(std::format("Client handling error: {}", err.what()));
        }

        close(client);
    }
}

void Server::handle_login(const Stream& stream)
{
    while (true) {
        auto message = stream.receive_msg();

        if (message["action"] != "login") {
            logger.info(std::format("Expect to get 'login' action. Get {} instead.", message["action"]));
            continue;
        }

        auto user = message["user"], password = message["password"];
        stream.send_msg(std::format("status:{}", Status::SUCCESS));
        logger.info(std::format("User {} successfully logged in.", message["user"]));
        return;
    }
}
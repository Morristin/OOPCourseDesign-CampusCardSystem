#include "Stream.h"

#include "../logger/Logger.h"

#include <sys/socket.h>

auto logger = Logger(__FILE__);

void Stream::send_msg(const std::string& msg) const
{
    if (const auto status = send(socket, msg.c_str(), msg.length(), 0); status < 0) {
        logger.error("Send message failed.");
        std::cout << "Send message failed. Please check connection and try again later." << std::endl;
    }
}

[[nodiscard]] Parser Stream::receive_msg() const
{
    std::vector<char> buffer(BUFFER_SIZE);
    const auto received_bytes = recv(socket, buffer.data(), BUFFER_SIZE, 0);

    if (received_bytes < 0) {
        logger.error("Receive message failed.");
        std::cout << "Receive message failed. Please check connection and try again later." << std::endl;
    } else if (received_bytes == 0) {
        logger.warning("Net connection is closed by server.");
        std::cout << "Net connection is closed by server. Program stopped." << std::endl;
        exit(0);
    }

    return Parser(std::string(buffer.data(), received_bytes));
}
#include "Stream.h"

#include "../logger/Logger.h"
#include "protocol.h"

#include <sys/socket.h>

static auto logger = Logger(__FILE__);

void Stream::send_msg(const std::string& msg) const
{
    // Send the length of message before actually send the body of message.
    const uint32_t net_length = htonl(static_cast<uint32_t>(msg.length()));

    if (send(socket, &net_length, sizeof(net_length), 0) < 0)
        logger.error("Send message failed. The length of message haven't been sent.");
    else if (const auto status = send(socket, msg.c_str(), msg.length(), 0); status < 0)
        logger.error("Send message failed. The length of message have been set.");
    else
        logger.debug(std::format("Send message: {}", msg));
}

[[nodiscard]] Parser Stream::receive_msg() const
{
    auto read_fixed_length_msg = [this](char* buffer, const size_t length) -> bool {
        size_t total_received = 0;
        while (total_received < length) {
            if (const auto bytes = recv(socket, buffer + total_received, length - total_received, 0); bytes > 0)
                total_received += bytes;
            else {
                logger.warning((bytes == 0) ? "Net connection is closed by opposite." : "Receive message failed.");
                return false;
            }
        }
        return true;
    };

    uint32_t net_length;
    if (!read_fixed_length_msg(reinterpret_cast<char*>(&net_length), sizeof(net_length)))
        return Parser(std::format(STATUS_WITH_MSG, MsgStatus::FAILED, ErrorMsg::NETWORK_ERROR));

    const uint32_t msg_length = ntohl(net_length);
    std::vector<char> buffer(msg_length);
    if (!read_fixed_length_msg(buffer.data(), msg_length))
        return Parser(std::format(STATUS_WITH_MSG, MsgStatus::FAILED, ErrorMsg::NETWORK_ERROR));

    return Parser(std::string(buffer.data(), msg_length));
}
#ifndef STREAM_H
#define STREAM_H

#include "Parser.h"

#include <string>

struct StreamException : std::exception {
private:
    std::string_view message;

public:
    explicit StreamException(const std::string_view message) : message(message) { }
    [[nodiscard]] const char* what() const noexcept override { return message.data(); }
};

class Stream {
private:
    static constexpr auto BUFFER_SIZE = 1024;

    int socket;

public:
    explicit Stream(const int fd) { this->socket = fd; }

    void send_msg(const std::string& msg) const;
    [[nodiscard]] Parser receive_msg() const;
};

#endif

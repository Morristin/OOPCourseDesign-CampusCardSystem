#ifndef STREAM_H
#define STREAM_H

#include "Parser.h"

#include <string>

class Stream {
private:
    int socket;

public:
    explicit Stream(const int fd) { this->socket = fd; }

    void send_msg(const std::string& msg) const;
    [[nodiscard]] Parser receive_msg() const;
};

#endif

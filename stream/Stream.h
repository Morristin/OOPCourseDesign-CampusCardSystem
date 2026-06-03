#ifndef STREAM_H
#define STREAM_H

#include <string>

class Stream {
private:
    static constexpr auto BUFFER_SIZE = 1024;

    int socket;

public:
    explicit Stream(const int fd) { this->socket = fd; }

    void send_msg(const std::string& msg) const;
    [[nodiscard]] std::string receive_msg() const;
};

#endif

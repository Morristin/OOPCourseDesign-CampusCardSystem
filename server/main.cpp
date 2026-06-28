#include "Server.h"

[[noreturn]] int main()
{
    auto server = Server();
    while (true)
        server.run();
}
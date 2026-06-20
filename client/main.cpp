#include "Client.h"

#include <iostream>

int main()
{
    std::cout << "Welcome to Campus Card Management System." << std::endl;

    const auto client = Client().start();
}
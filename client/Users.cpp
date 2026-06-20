#include "Users.h"

#include <iostream>

void User::show_dashboard()
{
    while (true) {
        auto current_dashboard = dashboard;

        for (int index = 1; index <= current_dashboard.length(); index++)
            std::cout << index << ". " << current_dashboard[index].name << ": " << current_dashboard[index].description << std::endl;

        char choice = '0';
        while (!(choice >= '1' && (choice - '0') <= current_dashboard.length())) {
            std::cin >> choice;

            if (choice != 'b' && choice != 'o' && choice != 'q' && choice != 'h' && (choice < '0' || choice > '9'))
                std::cout << "The input character is invalid. Enter 'h' for help.";
            else if (choice >= '0' && choice <= '9' && (choice == '0' || (choice - '0') > current_dashboard.length()))
                std::cout << std::format("The input number is invalid. You can only type 1 - {}", current_dashboard.length());
        }

        // Special options: Navigate Back, Quit and Help.
        switch (choice) {
        case 'b':
            current_dashboard = current_dashboard.previous_dashboard();
            continue;
        case 'q':
            std::cout << std::format("Shutting down software. Goodbye, {}.", user_information.username);
            exit(0);
        case 'h':
            std::cout << "Simple User Manual: " << std::endl;
            std::cout << "Type 'b' to navigate back to previous dashboard." << std::endl;
            std::cout << "Type 'q' to shut down this software." << std::endl;
            continue;
        default:;
        }

        // Execute the action of menuitem or dive into deeper dashboard.
        if (std::holds_alternative<Dashboard*>(current_dashboard[choice - '0'].value))
            current_dashboard = *std::get<Dashboard*>(current_dashboard[choice - '0'].value);
        else
            std::get<std::function<void()>>(current_dashboard[choice - '0'].value)();
    }
}
#include "Users.h"

#include <iostream>

void User::show_dashboard() const
{
    auto current_dashboard = main_dashboard();

    while (true) {
        std::cout << "Enter the number index before commands to execute them. Enter 'h' for help." << std::endl;
        for (int index = 1; index <= current_dashboard.length(); index++)
            std::cout << index << ". " << current_dashboard[index].name << ": " << current_dashboard[index].description << std::endl;

        std::string choice;
        while (true) {
            std::cin >> choice;

            if (choice.size() != 1 || choice != "b" && choice != "q" && choice != "h" && (choice < "0" || choice > "9"))
                std::cout << "The input character is invalid. Enter 'h' for help." << std::endl;
            else if (choice >= "0" && choice <= "9" && (choice == "0" || std::stoi(choice) > current_dashboard.length()))
                std::cout << std::format("The input number is invalid. You can only type 1 - {}", current_dashboard.length()) << std::endl;
            else
                break;
        }

        // Special options: Navigate Back, Quit and Help.
        switch (choice[0]) {
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
        if (std::holds_alternative<Dashboard*>(current_dashboard[std::stoi(choice)].value))
            current_dashboard = *std::get<Dashboard*>(current_dashboard[std::stoi(choice)].value);
        else
            std::get<std::function<void()>>(current_dashboard[std::stoi(choice)].value)();

        std::cout << std::endl;
    }
}
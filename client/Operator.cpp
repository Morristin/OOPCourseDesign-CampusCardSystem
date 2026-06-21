#include "../stream/protocol.h"
#include "Users.h"

#include <iostream>
#include <map>

void Operator::add_student() const
{
    std::string real_name, gender, student_id, department;
    std::cout << "Please enter the info of student (RealName, Gender, StudentID, Department):" << std::endl;
    std::cin >> real_name >> gender >> student_id >> department;

    client.send_msg(std::format(ACTION_ADD_STUDENT, real_name, gender, student_id, department));

    if (const auto response = client.receive_msg(); response["status"] == MsgStatus::SUCCESS)
        std::cout << OutputType::SUCCESS << "Student registered successfully." << OutputType::RESET << std::endl;
    else if (response["message"] == ErrorMsg::USERINFO_EXISTS)
        std::cout << OutputType::ERROR << "Student register Failed: StudentID already exists." << OutputType::RESET << std::endl;
    else if (response["message"] == ErrorMsg::USER_ALREADY_EXISTS)
        std::cout << OutputType::ERROR << "Student register Failed: A user with StudentID as username already exists." << OutputType::RESET << std::endl;
}

void Operator::add_multiple_student() const
{
    // Tool functions strip() and split().
    auto strip = [](const std::string& str) -> std::string {
        const std::size_t first = str.find_first_not_of(" \t\r\n");
        const std::size_t last = str.find_last_not_of(" \t\r\n");
        return (first == std::string::npos) ? "" : str.substr(first, last - first + 1);
    };

    auto parse_CSV_line = [&strip](const std::string& line, const char ch = ',') {
        std::vector<std::string> fields;

        std::size_t start = 0;
        std::size_t end = line.find(ch);

        while (end != std::string::npos) {
            fields.emplace_back(strip(line.substr(start, end - start)));
            start = end + 1;
            end = line.find(ch, start);
        }

        fields.emplace_back(strip(line.substr(start)));
        return fields;
    };

    // Main interact and connection.
    std::cout << "Please paste the CSV data here. Type a single char 'q' to end input." << std::endl;
    std::cout << OutputType::WARNING << "Please make sure that the format of CSV data is: name, gender, id and department. Separated by comma." << OutputType::RESET << std::endl;

    std::string line;
    int success_count = 0;
    std::map<std::string, std::string> failed_list;

    while (std::getline(std::cin >> std::ws, line)) {
        if (strip(line) == "q" || strip(line) == "Q")
            break;

        auto fields = parse_CSV_line(line);
        if (fields.size() != 4) {
            std::cout << OutputType::ERROR << std::format("The format of your line doesn't match requirement: {}", line) << OutputType::RESET << std::endl;
            continue;
        }

        client.send_msg(std::format(ACTION_ADD_STUDENT, fields[0], fields[1], fields[2], fields[3]));
        if (const auto response = client.receive_msg(); response["status"] == MsgStatus::SUCCESS)
            success_count++;
        else
            failed_list[fields[2]] = response["message"];
    }

    // Output the data failed to add into database.
    if (success_count > 0)
        std::cout << OutputType::SUCCESS << std::format("Successfully add {} students.", success_count) << OutputType::RESET << std::endl;
    if (!failed_list.empty()) {
        std::cout << OutputType::ERROR << std::format("Failed add following {} students: ", failed_list.size()) << OutputType::RESET << std::endl;
        for (const auto& [student_id, error_msg] : failed_list)
            std::cout << std::format("StudentID: {}, Error Message: {}.", student_id, error_msg) << std::endl;
    }
}

void Operator::del_student() const
{
    std::string student_id;
    std::cout << "Enter the StudentID to delete: " << std::endl;
    std::cin >> student_id;

    client.send_msg(std::format(ACTION_DEL_STUDENT, student_id));

    if (const auto response = client.receive_msg(); response["status"] == MsgStatus::SUCCESS)
        std::cout << OutputType::SUCCESS << "Student deleted successfully." << OutputType::RESET << std::endl;
    else if (response["message"] == ErrorMsg::USERINFO_NOT_FOUND)
        std::cout << OutputType::ERROR << "Failed to delete student. StudentID does not exist." << OutputType::RESET << std::endl;
}

void Operator::update_student() const
{
    std::string student_id, real_name, gender, department;
    std::cout << "Please enter the ID of student who you want to update: " << std::endl;
    std::cin >> student_id;

    std::cout << "Please enter the new RealName, Gender or Gender (enter '0' to keep previous info): " << std::endl;
    std::cin >> real_name >> gender >> department;

    auto format = [](const std::string& content) -> std::string { return (content == "0") ? "" : content; };
    client.send_msg(std::format(ACTION_UPDATE_STUDENT, student_id, format(real_name), format(gender), format(department)));

    if (const auto response = client.receive_msg(); response["status"] == MsgStatus::SUCCESS)
        std::cout << OutputType::SUCCESS << "Student updated successfully." << OutputType::RESET << std::endl;
    else if (response["message"] == ErrorMsg::USERINFO_NOT_FOUND)
        std::cout << OutputType::ERROR << "Failed to modify student. StudentID does not exist." << OutputType::RESET << std::endl;
}

void Operator::recharge() const
{
    std::string card_number, amount_string;
    std::cout << "Enter the card number and recharge amount: " << std::endl;
    std::cin >> card_number >> amount_string;

    try {
        double amount = std::stod(amount_string);
        client.send_msg(std::format(ACTION_RECHARGE, card_number, std::format("{:.2f}", amount)));
    } catch (const std::exception&) {
        std::cout << "Invalid recharge amount format. You should enter a valid float." << std::endl;
        return;
    }

    if (const auto response = client.receive_msg(); response["status"] == MsgStatus::SUCCESS)
        std::cout << OutputType::SUCCESS << "Recharge successful." << OutputType::RESET << std::endl;
    else if (response["message"] == ErrorMsg::CARD_NOT_FOUND)
        std::cout << OutputType::ERROR << "Failed to recharge. The card number does not exist." << OutputType::RESET << std::endl;
    else if (response["message"] == ErrorMsg::ACCOUNT_ABNORMAL)
        std::cout << OutputType::ERROR << "Failed to recharge. The account is frozen or deleted." << OutputType::RESET << std::endl;
}

void Operator::manage_accounts() const
{
    std::string username, status_choice;

    std::cout << "Enter the username of the account and the index of action you want to perform: " << std::endl;
    std::cout << "1, freeze account. 2, delete account. 3, restore account." << std::endl;
    std::cin >> username >> status_choice;

    if (status_choice == "1")
        client.send_msg(std::format(ACTION_UPDATE_STATUS, username, UserStatus::FROZEN));
    else if (status_choice == "2")
        client.send_msg(std::format(ACTION_UPDATE_STATUS, username, UserStatus::DELETED));
    else if (status_choice == "3")
        client.send_msg(std::format(ACTION_UPDATE_STATUS, username, UserStatus::NORMAL));
    else {
        std::cout << OutputType::ERROR << "Invalid choice. Please enter number between 1-3." << OutputType::RESET << std::endl;
        return;
    }

    if (const auto response = client.receive_msg(); response["status"] == MsgStatus::SUCCESS)
        std::cout << OutputType::SUCCESS << "Account status updated successfully." << OutputType::RESET << std::endl;
    else if (response["message"] == ErrorMsg::USER_NOT_FOUND)
        std::cout << OutputType::ERROR << "Failed to update states. Username does not exist." << OutputType::RESET << std::endl;
    else if (response["message"] == ErrorMsg::TARGET_NOT_STUDENT)
        std::cout << OutputType::ERROR << "Failed to update states. You can only manage student accounts." << OutputType::RESET << std::endl;
}
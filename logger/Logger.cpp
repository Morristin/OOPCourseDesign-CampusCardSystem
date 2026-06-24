#include "Logger.h"

#include "../protocol/colors.h"

#include <iostream>

// Initialize static class member std::mutex.
std::mutex Logger::log_mutex;

Logger::Logger(const std::string& file_name)
{
    const auto short_file_name = file_name.substr(file_name.rfind('/') + 1);
    module_name                = short_file_name.substr(0, short_file_name.rfind('.'));

    if (log_file.open(log_filename, std::ios::app); !log_file.is_open()) {
        std::cout << OutputType::CRITICAL << std::format("Error open log file: {}. Please check CMake settings and your permission.", log_filename) << OutputType::RESET << std::endl;
        exit(1);
    }
}

std::vector<std::string> Logger::to_vector()
{
    std::lock_guard lock(log_mutex);
    std::ifstream file(LOG_FILE);

    std::vector<std::string> vector_logs;
    std::string line;
    while (std::getline(file, line))
        vector_logs.emplace_back(line);
    return vector_logs;
}

void Logger::write(const std::string& level, const std::string& message)
{
    std::lock_guard lock(log_mutex);

    static constexpr auto date_format = "%b %d %H:%M:%S";
    static constexpr auto log_format  = "{:<9} {}: {}\n";

    const auto tt = time(nullptr);
    log_file << std::put_time(std::localtime(&tt), date_format) << " ";
    log_file << std::format(log_format, std::format("[{}]", level), module_name, message) << std::flush;

    if (level == "WARNING" && DEBUG_MODE)
        std::cout << OutputType::WARNING << message << OutputType::RESET << std::endl;
    else if (level == "ERROR" && DEBUG_MODE)
        std::cout << OutputType::ERROR << message << OutputType::RESET << std::endl;
    else if (level == "CRITICAL" && DEBUG_MODE)
        std::cout << OutputType::CRITICAL << "Serious error happens: " << message << OutputType::RESET << std::endl;
}
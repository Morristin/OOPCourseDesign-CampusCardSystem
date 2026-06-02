#include "Logger.h"

#include <iostream>

constexpr int LOG_OPEN_FAILED = 1;

Logger::Logger(const std::string& filename)
{
    log_name = filename;
    log_file.open(log_name);

    if (!log_file.is_open()) {
        std::cerr << "Error open log file: " << log_name << std::endl;
        exit(LOG_OPEN_FAILED);
    }
}

void Logger::write(const std::string& level, const std::string& msg)
{
    const auto tt = time(nullptr);
    log_file << std::put_time(std::localtime(&tt), date_format);
    log_file << std::format(log_format, level, msg);
}
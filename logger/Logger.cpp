#include "Logger.h"

#include <iostream>

constexpr int LOG_OPEN_FAILED = 1;

Logger::Logger(const std::string& file_name)
{
    const auto short_file_name = file_name.substr(file_name.rfind('/') + 1);
    module_name = short_file_name.substr(0, short_file_name.rfind('.'));

    if (log_file.open(log_name); !log_file.is_open()) {
        std::cerr << "Error open log file: " << log_name << std::endl;
        exit(LOG_OPEN_FAILED);
    }
}

void Logger::write(const std::string& level, const std::string& msg)
{
    const auto tt = time(nullptr);

    std::lock_guard lock(log_mutex);
    log_file << std::put_time(std::localtime(&tt), date_format);
    log_file << std::format(log_format, level, module_name, msg);
    log_file << std::flush;
}
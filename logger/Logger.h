#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <iostream>
#include <mutex>
#include <string>

class Logger {
private:
    static constexpr std::string log_name = LOG_FILE;
    std::string module_name;

    static std::mutex log_mutex;
    std::ofstream log_file;

    static constexpr auto log_format = " [{}]\n{}: {}\n";
    static constexpr auto date_format = "%b %d %H:%M:%S";
    void write(const std::string& level, const std::string& message);

public:
    explicit Logger(const std::string& file_name);
    ~Logger() { log_file.close(); }

    void debug(const std::string& msg) { write("DEBUG", msg); }
    void info(const std::string& msg) { write("INFO", msg); }
    void warning(const std::string& msg) { write("WARNING", msg); }
    void error(const std::string& msg) { write("ERROR", msg); }
    void critical(const std::string& msg) { write("CRITICAL", msg); }
};

#endif

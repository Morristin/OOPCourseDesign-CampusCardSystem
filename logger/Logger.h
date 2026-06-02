#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <string>

class Logger {
private:
    static constexpr std::string log_name = "log.log";
    std::string module_name;
    std::ofstream log_file;

    static constexpr auto log_format = " [{}]\n{}: {}";
    static constexpr auto date_format = "%b %d %H:%M:%S";
    void write(const std::string& level, const std::string& msg);

public:
    explicit Logger(const std::string& file_name);
    ~Logger() { log_file.close(); }

    void debug(const std::string& msg) { write("DEBUG", msg); }
    void info(const std::string& msg) { write("INFO", msg); }
    void warning(const std::string& msg) { write("WARNING", msg); }
    void error(const std::string& msg) { write("ERROR", msg); }
};

#endif

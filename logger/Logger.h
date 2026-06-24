#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <mutex>
#include <string>

class Logger {
private:
    static std::mutex log_mutex;

    static constexpr std::string log_filename = LOG_FILE;
    std::ofstream log_file;

    std::string module_name;
    void write(const std::string& level, const std::string& message);

public:
    explicit Logger(const std::string& file_name);
    ~Logger() { log_file.close(); }

    static std::vector<std::string> to_vector();

    void debug(const std::string& msg) { write("DEBUG", msg); }
    void info(const std::string& msg) { write("INFO", msg); }
    void warning(const std::string& msg) { write("WARNING", msg); }
    void error(const std::string& msg) { write("ERROR", msg); }
    void critical(const std::string& msg) { write("CRITICAL", msg); }
};

#endif

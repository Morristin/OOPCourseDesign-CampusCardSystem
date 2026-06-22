#pragma once

#include <string>
#include <vector>

inline std::string strip(const std::string& str)
{
    const std::size_t first = str.find_first_not_of(" \t\r\n");
    const std::size_t last = str.find_last_not_of(" \t\r\n");
    return (first == std::string::npos) ? "" : str.substr(first, last - first + 1);
};

inline std::vector<std::string> parse_CSV_line(const std::string& line, const char ch = ',')
{
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
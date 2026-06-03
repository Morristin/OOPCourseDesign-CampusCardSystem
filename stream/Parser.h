#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <unordered_map>

class Parser {
private:
    std::string origin_msg;
    std::unordered_map<std::string, std::string> data;
    void store_item(const std::string& item);

public:
    explicit Parser(const std::string& origin_msg);
    std::string origin() { return origin_msg; }

    // Operator[] won't throw any exceptions. Return blank string if key can not be found.
    std::string operator[](const std::string& key);
};

#endif

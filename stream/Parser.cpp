#include "Parser.h"

void Parser::store_item(const std::string& item)
{
    const auto colon_pos = item.find(':');
    data[item.substr(0, colon_pos)] = item.substr(colon_pos + 1);
}

Parser::Parser(const std::string& origin_msg)
{
    this->origin_msg = origin_msg;

    size_t start, end = 0;
    while ((end = origin_msg.find(',', start = (end == 0 ? 0 : end + 1))) != std::string::npos) {
        store_item(origin_msg.substr(start, end - start));
    }
    store_item(origin_msg.substr(start));
}

std::string Parser::operator[](const std::string& key)
{
    const auto item = data.find(key);
    return (item != data.end()) ? item->second : "";
}
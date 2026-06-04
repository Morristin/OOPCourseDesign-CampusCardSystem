#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <string_view>

namespace ActionStatus {
constexpr std::string_view SUCCESS = "success";
constexpr std::string_view FAILED = "failed";
}

namespace ErrorMsg {
constexpr std::string_view USER_NOT_FOUND = "UserNotFound";
constexpr std::string_view WRONG_PASSWORD = "WrongPassword";
}

#endif

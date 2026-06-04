#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <string_view>

constexpr auto ACTION_LOGIN = "action:login,username:{},password:{}";

constexpr auto STATUS_ONLY = "status:{}";
constexpr auto STATUS_WITH_MEG = "status:{},message:{}";

namespace Status {
constexpr std::string_view SUCCESS = "success";
constexpr std::string_view FAILED = "failed";
}

namespace ErrorMsg {
constexpr std::string_view USER_NOT_FOUND = "UserNotFound";
constexpr std::string_view WRONG_PASSWORD = "WrongPassword";
constexpr std::string_view SQL_INJECTION = "SQLInjection";
}

#endif

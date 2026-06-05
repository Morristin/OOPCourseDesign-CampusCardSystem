#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <string_view>

namespace Status {
constexpr std::string_view SUCCESS = "success";
constexpr std::string_view FAILED = "failed";
}

namespace ErrorMsg {
constexpr std::string_view USER_NOT_FOUND = "UserNotFound";
constexpr std::string_view WRONG_PASSWORD = "WrongPassword";
constexpr std::string_view SQL_INJECTION = "SQLInjection";
}

constexpr auto ACTION_LOGIN = "action:login,username:{},password:{}";
constexpr auto FAILED_WITH_MSG = "status:{},message:{}";

constexpr auto LOGIN_USER_STATUS = "status:{},permission:{},status:{},card_number:{}";
struct LoginUserStatus {
    const std::string_view permission;
    const std::string_view status;
    const std::string_view card_number;
    [[nodiscard]] std::string message() const { return std::format(LOGIN_USER_STATUS, Status::SUCCESS, permission, status, card_number); }
};

#endif

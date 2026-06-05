#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <format>
#include <string_view>

namespace MsgStatus {
constexpr std::string_view SUCCESS = "success";
constexpr std::string_view FAILED = "failed";
}

namespace ErrorMsg {
constexpr std::string_view USER_NOT_FOUND = "UserNotFound";
constexpr std::string_view WRONG_PASSWORD = "WrongPassword";
constexpr std::string_view FINDING_FAILED = "FailedToFindUser";
}

namespace Permission {
constexpr int User = 4;
constexpr int OPERATOR = 6;
constexpr int ADMIN = 7;
}

namespace UserStatus {
constexpr int NORMAL = 0;
constexpr int DELETED = -1;
constexpr int FROZEN = 6;
}

constexpr auto ACTION_LOGIN = "action:login,username:{},password:{}";
constexpr auto FAILED_WITH_MSG = "status:{},message:{}";

constexpr auto LOGIN_USER_STATUS = "status:{},permission:{},user_status:{},card_number:{}";
struct LoginUserStatus {
    const std::string_view permission;
    const std::string_view status;
    const std::string_view card_number;
    [[nodiscard]] std::string message() const { return std::format(LOGIN_USER_STATUS, MsgStatus::SUCCESS, permission, status, card_number); }
};

#endif

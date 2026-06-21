#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <format>
#include <string_view>

namespace MsgStatus {
constexpr std::string_view SUCCESS = "success";
constexpr std::string_view FAILED = "failed";
}

namespace ErrorMsg {
constexpr std::string_view UNKNOWN_ACTION = "UnknownAction";
constexpr std::string_view PERMISSION_DENIED = "PermissionDenied";

constexpr std::string_view USER_NOT_FOUND = "UserNotFound";
constexpr std::string_view USER_ALREADY_EXISTS = "UserAlreadyExists";
constexpr std::string_view CARD_NOT_FOUND = "CardNotFound";
constexpr std::string_view ACCOUNT_ABNORMAL = "AccountAbnormal";

constexpr std::string_view PASSWORD_WRONG = "PasswordWrong";
constexpr std::string_view BALANCE_INSUFFICIENT = "BalanceInsufficient";
}

namespace Permission {
constexpr int DEFAULT = 0;
constexpr int STUDENT = 4;
constexpr int OPERATOR = 6;
constexpr int SUPEROPERATOR = 7;
}

namespace UserStatus {
constexpr int NORMAL = 0;
constexpr int DELETED = -1;
constexpr int FROZEN = 6;
}

constexpr auto STATUS_WITH_MSG = "status:{},message:{}";

constexpr auto ACTION_LOGIN = "action:login,username:{},password:{}";
constexpr auto LOGIN_USER_STATUS = "status:{},username:{},permission:{},user_status:{},card_number:{}";
struct LoginUserStatus {
    const std::string_view username;
    const std::string_view permission;
    const std::string_view status;
    const std::string_view card_number;
    [[nodiscard]] std::string message() const { return std::format(LOGIN_USER_STATUS, MsgStatus::SUCCESS, username, permission, status, card_number); }
};

constexpr auto ACTION_ADD_OPERATOR = "action:add_operator,username:{},password:{}";
constexpr auto ACTION_DELETE_OPERATOR = "action:del_operator,username:{}";

constexpr auto ACTION_RECHARGE = "action:recharge,card_number:{},amount:{}";

constexpr auto ACTION_CONSUME = "action:consume,card_number:{},amount:{},merchant:{}";

#endif

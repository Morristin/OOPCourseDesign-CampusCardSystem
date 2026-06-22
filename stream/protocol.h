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
constexpr std::string_view USERINFO_NOT_FOUND = "UserInfoNotFound";
constexpr std::string_view USERINFO_EXISTS = "UserInfoAlreadyExists";
constexpr std::string_view CARD_NOT_FOUND = "CardNotFound";

constexpr std::string_view ACCOUNT_ABNORMAL = "AccountAbnormal";
constexpr std::string_view BALANCE_INSUFFICIENT = "BalanceInsufficient";
constexpr std::string_view PASSWORD_WRONG = "PasswordWrong";
constexpr std::string_view TARGET_NOT_STUDENT = "TargetNotStudent";
}

namespace Password {
constexpr std::string DEFAULT = "123456";
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

namespace CardNumber {
constexpr std::string BLANK = "NULL";
}

constexpr auto STATUS_WITH_MSG = "status:{},message:{}";

constexpr auto LONG_MSG_START = "status:{},message:START,length:{}";
constexpr auto LONG_MSG_END = "status:{},message:END,length:{}";

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

constexpr auto ACTION_ADD_STUDENT = "action:add_student,real_name:{},gender:{},student_id:{},department:{}";
constexpr auto ACTION_DEL_STUDENT = "action:del_student,student_id:{}";
constexpr auto ACTION_UPDATE_STUDENT = "action:update_student,student_id:{},real_name:{},gender:{},department:{}";

constexpr auto ACTION_RECHARGE = "action:recharge,card_number:{},amount:{}";
constexpr auto ACTION_UPDATE_STATUS = "action:update_status,username:{},status:{}";

constexpr auto ACTION_CONSUME = "action:consume,card_number:{},amount:{},merchant:{}";

constexpr auto ACTION_QUERY_TRANSACTION = "action:query_transaction,card_number:{}";

constexpr auto TRANSACTION_RECORD = "time:{},amount:{},balance:{},operator:{}";

#endif

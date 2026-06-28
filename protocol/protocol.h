#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <format>
#include <string_view>

namespace MsgStatus {
constexpr std::string_view SUCCESS = "success";
constexpr std::string_view FAILED  = "failed";
}

namespace ErrorMsg {
constexpr std::string_view NETWORK_ERROR = "NetworkError";

constexpr std::string_view UNKNOWN_ACTION    = "UnknownAction";
constexpr std::string_view PERMISSION_DENIED = "PermissionDenied";

constexpr std::string_view USER_NOT_FOUND     = "UserNotFound";
constexpr std::string_view USER_EXISTS        = "UserExists";
constexpr std::string_view USERINFO_NOT_FOUND = "UserInfoNotFound";
constexpr std::string_view USERINFO_EXISTS    = "UserInfoAlreadyExists";
constexpr std::string_view CARD_NOT_FOUND     = "CardNotFound";

constexpr std::string_view ACCOUNT_ABNORMAL = "AccountAbnormal";
constexpr std::string_view ACCOUNT_DELETED  = "AccountDeleted";
constexpr std::string_view ACCOUNT_FROZEN   = "AccountFrozen";

constexpr std::string_view BALANCE_INSUFFICIENT  = "BalanceInsufficient";
constexpr std::string_view PASSWORD_WRONG        = "PasswordWrong";
constexpr std::string_view TARGET_NOT_STUDENT    = "TargetNotStudent";
constexpr std::string_view TRANSACTION_NOT_FOUND = "TransactionNotFound";
}

namespace Password {
constexpr std::string DEFAULT = "123456";
}

namespace Permission {
constexpr int DEFAULT       = 0;
constexpr int STUDENT       = 4;
constexpr int OPERATOR      = 6;
constexpr int SUPEROPERATOR = 7;
}

namespace UserStatus {
constexpr int NORMAL  = 0;
constexpr int DELETED = -1;
constexpr int FROZEN  = 6;
}

namespace CardNumber {
constexpr std::string_view BLANK = "NULL";
}

namespace SystemSettings {
constexpr std::string_view FIXED_FEE = "FIXED_FEE";
}

constexpr std::string_view LONG_MSG_START = "status:{},message:START,length:{}";
constexpr std::string_view LONG_MSG_END   = "status:{},message:END,length:{}";

constexpr std::string_view STATUS_WITH_MSG = "status:{},message:{}";
constexpr std::string_view LOGIN_RESULT    = "status:{},username:{},permission:{},user_status:{},card_number:{}";

namespace Action {
constexpr std::string_view LOGIN                  = "action:login,username:{},password:{}";
constexpr std::string_view UPDATE_SYSTEM_SETTINGS = "action:update_system_settings,key:{},value:{}";
constexpr std::string_view EXPORT_SERVER_LOGS     = "action:export_server_logs";

constexpr std::string_view CREATE_OPERATOR = "action:create_operator,username:{},password:{}";
constexpr std::string_view DELETE_OPERATOR = "action:delete_operator,username:{}";

constexpr std::string_view CREATE_STUDENT          = "action:create_student,real_name:{},gender:{},student_id:{},department:{}";
constexpr std::string_view DELETE_STUDENT          = "action:delete_student,student_id:{}";
constexpr std::string_view UPDATE_STUDENT_STATUS   = "action:update_student_status,username:{},status:{}";
constexpr std::string_view UPDATE_STUDENT_USERINFO = "action:update_student_userinfo,student_id:{},real_name:{},gender:{},department:{}";

constexpr std::string_view RECHARGE = "action:recharge,card_number:{},amount:{}";
constexpr std::string_view CONSUME  = "action:consume,card_number:{},amount:{},merchant:{}";

constexpr std::string_view QUERY_TRANSACTION       = "action:query_transaction,card_number:{}";
constexpr std::string_view QUERY_OWN_TRANSACTION   = "action:query_own_transaction";
constexpr std::string_view QUERY_ABNORMAL_ACCOUNTS = "action:query_abnormal_accounts";
constexpr std::string_view EXPORT_TRANSACTION      = "action:export_transaction";
constexpr std::string_view GENERATE_STATISTICS     = "action:generate_statistics,type:{}";
}

constexpr std::string_view LOG_LINE = "content:{}";

constexpr std::string_view DB_ABNORMAL_ACCOUNT       = "username:{},card_number:{},status:{}";
constexpr std::string_view DB_USER_INFO              = "password:{},permission:{},status:{},card_number:{}";
constexpr std::string_view DB_TRANSACTION_STATISTICS = "category:{},consumption:{},count:{}";
constexpr std::string_view DB_TRANSACTION_RECORD     = "time:{},amount:{},balance:{},operator:{}";

#endif

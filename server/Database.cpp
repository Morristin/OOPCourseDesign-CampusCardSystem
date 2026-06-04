#include "Database.h"

#include "../logger/Logger.h"

static auto logger = Logger(__FILE__);

Database::Database(const std::string& database_path)
{
    if (const int status = sqlite3_open(database_path.c_str(), &database); status != SQLITE_OK) {
        logger.critical(std::format("Can not connect to database: {}", database_path));
    }
}
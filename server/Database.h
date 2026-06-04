#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include <string>

class Database {
private:
    static constexpr std::string DATABASE_PATH = "server/data.sqlite3";
    sqlite3* database { };
    sqlite3_stmt* cursor { };

public:
    explicit Database(const std::string& database_path);
    Database() : Database(DATABASE_PATH) { };
    ~Database() { sqlite3_close(database); }

    void initialize() const;
    std::string_view check_identity(const std::string& username, const std::string& password);
};

#endif

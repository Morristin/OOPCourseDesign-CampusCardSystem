#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include <string>

class Database {
private:
    static constexpr std::string DATABASE_PATH = "server/data.sqlite3";
    sqlite3* database { };

public:
    explicit Database(const std::string& database_path);
    Database() : Database(DATABASE_PATH) { };
    ~Database() { sqlite3_close(database); }
};

#endif

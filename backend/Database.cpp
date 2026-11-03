#include "Database.h"

#include <stdexcept>
#include <string>

Database::Database(const std::string &path)
{
    const int rc = sqlite3_open(
        path.c_str(),
        &db_);

    if (rc != SQLITE_OK)
    {
        std::string message = db_ ? sqlite3_errmsg(db_) : "Could not allocate SQLite connection";

        sqlite3_close(db_);
        db_ = nullptr;

        throw std::runtime_error(message);
    }

    try
    {
        check(sqlite3_busy_timeout(db_, 5000));
    }
    catch (...)
    {
        sqlite3_close(db_);
        db_ = nullptr;
        throw;
    }
}

Database::~Database()
{
    if (db_)
    {
        sqlite3_close(db_);
    }
}

sqlite3 *Database::handle() const noexcept
{
    return db_;
}

void Database::check(int result) const
{
    if (result != SQLITE_OK)
    {
        throw std::runtime_error(sqlite3_errmsg(db_));
    }
}

void Database::execute(const std::string &sql)
{
    char *errorMessage = nullptr;

    const int rc = sqlite3_exec(
        db_,
        sql.c_str(),
        nullptr,
        nullptr,
        &errorMessage);

    if (rc != SQLITE_OK)
    {
        const std::string message = errorMessage ? errorMessage : sqlite3_errmsg(db_);

        sqlite3_free(errorMessage);

        throw std::runtime_error(message);
    }
}
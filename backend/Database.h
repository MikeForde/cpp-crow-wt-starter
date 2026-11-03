#pragma once

#include <sqlite3.h>
#include <string>

class Database
{
public:
    // constructor - prevent implicit initialisation
    explicit Database(const std::string &path);
    // destructor
    ~Database();

    // Prevent copying
    Database(const Database &) = delete;
    // Prevent reassignment
    Database &operator=(const Database &) = delete;

    // Non-owning pointer --> Statement will need handle but must never close it
    sqlite3 *handle() const noexcept;

    void execute(const std::string &sql);

    void check(int result) const;

private:
    sqlite3 *db_ = nullptr;
};
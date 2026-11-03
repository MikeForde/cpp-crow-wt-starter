#pragma once

#include "Database.h"

#include <sqlite3.h>
#include <string>

class Statement {
    public:
        Statement(Database& database, const std::string& sql);
        ~Statement();

        // Usual prevention of copy and assignment
        Statement(const Statement&) = delete;
        Statement& operator=(const Statement&) = delete;

        void bindInt(int index, int value);
        void bindText(int index, const std::string& value);

        // Returns true for a row, false when finished
        bool step();

        int columnInt(int index) const;
        std::string columnText(int index) const;

    private:
        Database& database_;
        sqlite3_stmt* stmt_ = nullptr;

};
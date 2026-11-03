#include "Statement.h"

#include <stdexcept>

Statement::Statement (Database& database,const std::string& sql) : database_(database) {
    sqlite3_stmt* raw = nullptr;

    const int rc = sqlite3_prepare_v2(
        database_.handle(),
        sql.c_str(),
        -1,
        &raw,
        nullptr
    );

    if (rc != SQLITE_OK) {
        sqlite3_finalize(raw);
        throw std::runtime_error(
            sqlite3_errmsg(database_.handle())
        );
    }

    stmt_ = raw;
}

Statement::~Statement() {
    if (stmt_) {
        sqlite3_finalize(stmt_);
    }
}

void Statement::bindInt(int index, int value) {
    database_.check(
        sqlite3_bind_int(stmt_, index, value)
    );
}

void Statement::bindText(int index, const std::string& value) {
    database_.check(
        sqlite3_bind_text(
            stmt_,
            index,
            value.c_str(),
            static_cast<int>(value.size()),
            SQLITE_TRANSIENT
        )
    );
}

bool Statement::step() {
    const int rc = sqlite3_step(stmt_);

    if (rc == SQLITE_ROW) {
        return true;
    }

    if (rc == SQLITE_DONE) {
        return false;
    }

    throw std::runtime_error(sqlite3_errmsg(database_.handle()));
}

int Statement::columnInt(int index) const {
    return sqlite3_column_int(stmt_, index);
}

std::string Statement::columnText(int index) const {
    const unsigned char* text = sqlite3_column_text(stmt_, index);

    if (!text) {
        if (sqlite3_column_type(stmt_, index) == SQLITE_NULL) {
            return "";
        }

        throw std::runtime_error("Could not read SQLITE text column");
    }

    const int length = sqlite3_column_bytes(stmt_, index);

    return std::string(
        reinterpret_cast<const char*>(text),
        static_cast<std::size_t>(length)
    );
}


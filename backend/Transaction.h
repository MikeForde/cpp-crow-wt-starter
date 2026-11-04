#pragma once

#include "Database.h"

class Transaction
{
public:
    explicit Transaction(Database &database);
    ~Transaction() noexcept;

    Transaction(const Transaction &) = delete;
    Transaction &operator=(const Transaction &) = delete;

    void commit();

private:
    Database &database_;
    bool committed_ = false;
};
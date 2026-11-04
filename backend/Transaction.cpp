#include "Transaction.h"

Transaction::Transaction(Database &database) : database_(database) {
    database_.execute("BEGIN IMMEDIATE");
}

Transaction::~Transaction() noexcept {
    if (!committed_ ) {
        try {
            database_.execute("ROLLBACK");
        } catch (...) {
            // ignore exceptions in destructor
        }   
    }
}

void Transaction::commit() {
    database_.execute("COMMIT");

    committed_ = true;
}
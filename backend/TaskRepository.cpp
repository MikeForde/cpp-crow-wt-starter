#include "TaskRepository.h"

#include <algorithm>
#include <optional>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

namespace
{

    using Statement = std::unique_ptr<
        sqlite3_stmt,
        decltype(&sqlite3_finalize)>;

    Statement prepare(sqlite3 *db, const char *sql)
    {
        sqlite3_stmt *raw = nullptr;

        const int rc = sqlite3_prepare_v2(
            db, sql, -1, &raw, nullptr);

        if (rc != SQLITE_OK)
        {
            if (raw)
            {
                sqlite3_finalize(raw);
            }

            throw std::runtime_error(sqlite3_errmsg(db));
        }

        return Statement(raw, sqlite3_finalize);
    }

    void checkDone(sqlite3 *db, int rc)
    {
        if (rc != SQLITE_DONE)
        {
            throw std::runtime_error(sqlite3_errmsg(db));
        }
    }

} // anonymous namespace

TaskRepository::TaskRepository(
    const std::string &dbPath)
{
    const int rc = sqlite3_open(
        dbPath.c_str(),
        &db_);

    if (rc != SQLITE_OK)
    {
        std::string error = db_
                                ? sqlite3_errmsg(db_)
                                : "Could not allocate SQLite connection";

        if (db_)
        {
            sqlite3_close(db_);
            db_ = nullptr;
        }

        throw std::runtime_error(error);
    }

    try
    {
        sqlite3_busy_timeout(db_, 5000);
        createSchema();
    }
    catch (...)
    {
        sqlite3_close(db_);
        db_ = nullptr;
        throw;
    }
}

TaskRepository::~TaskRepository()
{
    if (db_)
    {
        sqlite3_close(db_);
    }
}

void TaskRepository::createSchema()
{

    const char *sql = R"(
        CREATE TABLE IF NOT EXISTS tasks (
            id INTEGER PRIMARY KEY,
            title TEXT NOT NULL,
            completed INTEGER NOT NULL DEFAULT 0
                CHECK (completed IN (0, 1))
        );
    )";

    char *errorMessage = nullptr;

    const int rc = sqlite3_exec(
        db_,
        sql,
        nullptr,
        nullptr,
        &errorMessage);

    if (rc != SQLITE_OK)
    {
        std::string error = errorMessage
                                ? errorMessage
                                : sqlite3_errmsg(db_);

        sqlite3_free(errorMessage);

        throw std::runtime_error(error);
    }
}

Task TaskRepository::addTask(const std::string &title)
{

    std::lock_guard<std::mutex> lock(mutex_);

    auto stmt = prepare(
        db_,
        "INSERT INTO tasks (title, completed) "
        "VALUES (?, 0)");

    const int rc = sqlite3_bind_text(
        stmt.get(),
        1,
        title.c_str(),
        -1,
        SQLITE_TRANSIENT);

    if (rc != SQLITE_OK)
    {
        throw std::runtime_error(sqlite3_errmsg(db_));
    }

    checkDone(db_, sqlite3_step(stmt.get()));

    const int id = static_cast<int>(
        sqlite3_last_insert_rowid(db_));

    return Task{id, title, false};
}

std::vector<Task> TaskRepository::getAllTasks() const
{

    std::lock_guard<std::mutex> lock(mutex_);

    auto stmt = prepare(
        db_,
        "SELECT id, title, completed "
        "FROM tasks ORDER BY id");

    std::vector<Task> tasks;

    int rc;

    while ((rc = sqlite3_step(stmt.get())) == SQLITE_ROW)
    {

        const int id =
            sqlite3_column_int(stmt.get(), 0);

        const unsigned char *text =
            sqlite3_column_text(stmt.get(), 1);

        const bool completed =
            sqlite3_column_int(stmt.get(), 2) != 0;

        Task task{
            id,
            text ? reinterpret_cast<const char *>(text) : "",
            completed};

        tasks.push_back(std::move(task));
    }

    if (rc != SQLITE_DONE)
    {
        throw std::runtime_error(sqlite3_errmsg(db_));
    }

    return tasks;
}

std::optional<Task> TaskRepository::updateTask(
    int id,
    bool completed)
{
    // Protect shared data against concurrent requests
    std::lock_guard<std::mutex> lock(mutex_);

    // First retrieve the existing task
    auto selectStmt = prepare(
        db_,
        "SELECT title FROM tasks WHERE id = ?");

    sqlite3_bind_int(selectStmt.get(), 1, id);

    const int rc = sqlite3_step(selectStmt.get());

    if (rc == SQLITE_DONE)
    {
        return std::nullopt;
    }

    if (rc != SQLITE_ROW)
    {
        throw std::runtime_error(sqlite3_errmsg(db_));
    }

    const unsigned char *text =
        sqlite3_column_text(selectStmt.get(), 0);

    const std::string title = text
                                  ? reinterpret_cast<const char *>(text)
                                  : "";

    // Release the SELECT statement before updating
    selectStmt.reset();

    auto updateStmt = prepare(
        db_,
        "UPDATE tasks SET completed = ? WHERE id = ?");

    sqlite3_bind_int(
        updateStmt.get(), 1, completed ? 1 : 0);

    sqlite3_bind_int(updateStmt.get(), 2, id);

    checkDone(db_, sqlite3_step(updateStmt.get()));

    return Task{id, title, completed};
}

bool TaskRepository::deleteTask(int id)
{

    std::lock_guard<std::mutex> lock(mutex_);

    auto stmt = prepare(
        db_,
        "DELETE FROM tasks WHERE id = ?"
    );

    sqlite3_bind_int(stmt.get(), 1, id);

    checkDone(db_, sqlite3_step(stmt.get()));

    return sqlite3_changes(db_) > 0;
}
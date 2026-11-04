#include "TaskRepository.h"

#include "Statement.h"
#include "Transaction.h"

#include <utility>

TaskRepository::TaskRepository(
    const std::string &dbPath) : database_(dbPath)
{
    createSchema();
}

void TaskRepository::createSchema()
{

  database_.execute(R"(
        CREATE TABLE IF NOT EXISTS tasks (
            id INTEGER PRIMARY KEY,
            title TEXT NOT NULL,
            completed INTEGER NOT NULL DEFAULT 0
                CHECK (completed IN (0, 1))
        );
    )");
}

Task TaskRepository::addTask(const std::string &title)
{

    std::lock_guard<std::mutex> lock(mutex_);

    Statement stmt(
        database_,
        "INSERT INTO tasks (title, completed) "
        "VALUES (?, 0)"
    );

    stmt.bindText(1, title);
    stmt.step();

    const int id = static_cast<int>(
        sqlite3_last_insert_rowid(database_.handle()));

    return Task{id, title, false};
}

std::vector<Task> TaskRepository::getAllTasks() const {

    std::lock_guard<std::mutex> lock(mutex_);

    Statement stmt(
        database_,
        "SELECT id, title, completed "
        "FROM tasks ORDER BY id"
    );

    std::vector<Task> tasks;

    while (stmt.step()) {

        Task task{
            stmt.columnInt(0),
            stmt.columnText(1),
            stmt.columnInt(2) != 0
        };

        tasks.push_back(std::move(task));
    }

    return tasks;
}

std::optional<Task> TaskRepository::updateTask(
    int id,
    bool completed
) {
    std::lock_guard<std::mutex> lock(mutex_);

    Transaction transaction(database_);

    std::string title;

    {
        Statement select(
            database_,
            "SELECT title FROM tasks WHERE id = ?"
        );

        select.bindInt(1, id);

        if (!select.step()) {
            return std::nullopt;
        }

        title = select.columnText(0);
    }

    {
        Statement update(
            database_,
            "UPDATE tasks "
            "SET completed = ? WHERE id = ?"
        );

        update.bindInt(1, completed ? 1 : 0);
        update.bindInt(2, id);

        update.step();
    }

    transaction.commit();

    return Task{id, title, completed};
}

bool TaskRepository::deleteTask(int id)
{

    std::lock_guard<std::mutex> lock(mutex_);

    Statement stmt(
        database_,
        "DELETE FROM tasks WHERE id = ?"
    );

    stmt.bindInt(1, id);
    stmt.step();

    return sqlite3_changes(database_.handle()) > 0;
}
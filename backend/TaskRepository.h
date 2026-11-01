#pragma once

#include "Task.h"

#include <sqlite3.h>

#include <mutex>
#include <string>
#include <vector>
#include <optional>

class TaskRepository
{
public:
    explicit TaskRepository(const std::string& dbPath);

    ~TaskRepository();

    // Prevent copying the repository
    TaskRepository(const TaskRepository&) = delete;
    TaskRepository& operator=(const TaskRepository&) = delete;

    Task addTask(const std::string &title);

    std::vector<Task> getAllTasks() const;

    std::optional<Task> updateTask(
        int id,
        bool completed);

    bool deleteTask(int id);

private:
    void createSchema();

    sqlite3* db_ = nullptr;

    mutable std::mutex mutex_;
};
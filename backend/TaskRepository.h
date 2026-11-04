#pragma once

#include "Database.h"
#include "Task.h"

#include <mutex>
#include <string>
#include <vector>
#include <optional>

class TaskRepository
{
public:
    explicit TaskRepository(const std::string& dbPath);

    // Prevent copying the repository
    // TaskRepository(const TaskRepository&) = delete;
    // TaskRepository& operator=(const TaskRepository&) = delete;

    Task addTask(const std::string &title);

    std::vector<Task> getAllTasks() const;

    std::optional<Task> updateTask(
        int id,
        bool completed);

    bool deleteTask(int id);

private:
    void createSchema();

    mutable std::mutex mutex_;
    mutable Database database_;
};
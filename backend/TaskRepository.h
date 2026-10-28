#pragma once

#include "Task.h"

#include <mutex>
#include <string>
#include <vector>
#include <optional>

class TaskRepository
{
public:
    Task addTask(const std::string &title);

    std::vector<Task> getAllTasks() const;

    std::optional<Task> updateTask(
        int id,
        bool completed);

private:
    mutable std::mutex mutex_;

    std::vector<Task> tasks_;

    int nextId_ = 1;
};
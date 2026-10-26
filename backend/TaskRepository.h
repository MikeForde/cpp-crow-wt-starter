#pragma once

#include "Task.h"

#include <mutex>
#include <string>
#include <vector>

class TaskRepository {
    public:
        Task addTask(const std::string& title);

        std::vector<Task> getAllTasks() const;

    private:
        mutable std::mutex mutex_;

        std::vector<Task> tasks_;

        int nextId_ = 1;
};
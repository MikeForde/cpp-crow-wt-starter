#include "TaskRepository.h"

Task TaskRepository::addTask(const std::string& title) {

    std::lock_guard<std::mutex> lock(mutex_);

    Task task {
        nextId_++,
        title,
        false
    };

    tasks_.push_back(task);

    return task;
}

std::vector<Task> TaskRepository::getAllTasks() const {

    std::lock_guard<std::mutex> lock(mutex_);

    return tasks_;
}
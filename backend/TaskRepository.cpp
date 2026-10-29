#include "TaskRepository.h"

#include <algorithm>
#include <optional>

Task TaskRepository::addTask(const std::string &title)
{

    std::lock_guard<std::mutex> lock(mutex_);

    Task task{
        nextId_++,
        title,
        false};

    tasks_.push_back(task);

    return task;
}

std::vector<Task> TaskRepository::getAllTasks() const
{

    std::lock_guard<std::mutex> lock(mutex_);

    return tasks_;
}

std::optional<Task> TaskRepository::updateTask(
    int id,
    bool completed)
{
    // Protect shared data against concurrent requests
    std::lock_guard<std::mutex> lock(mutex_);

    // Find the task by ID
    auto it = std::find_if(
        tasks_.begin(),
        tasks_.end(),
        [id](const Task &task)
        {
            return task.id == id;
        });

    // Task not found
    if (it == tasks_.end())
    {
        return std::nullopt;
    }

    // Update the existing task
    it->completed = completed;

    // Return a copy of the updated task
    return *it;
}

bool TaskRepository::deleteTask(int id) {

    std::lock_guard<std::mutex> lock(mutex_);

    // Move tasks matching the ID to the end
    auto newEnd = std::remove_if(
        tasks_.begin(),
        tasks_.end(),
        [id](const Task& task) {
            return task.id == id;
        }
    );

    // No matching task found
    if (newEnd == tasks_.end()) {
        return false;
    }

    // Erase the unwanted elements
    tasks_.erase(newEnd, tasks_.end());

    return true;
}
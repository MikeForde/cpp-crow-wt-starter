#include <catch2/catch_test_macros.hpp>

#include "TaskRepository.h"

TEST_CASE("A new repository contains no tasks", "[TaskRepository]"){
    TaskRepository repo(":memory:");

    const auto tasks = repo.getAllTasks();

    REQUIRE(tasks.empty());
}

TEST_CASE("A task can be created",
          "[TaskRepository]") {

    TaskRepository repository(":memory:");

    const Task task = repository.addTask("Learn Catch2");

    REQUIRE(task.id > 0);
    REQUIRE(task.title == "Learn Catch2");
    REQUIRE(task.completed == false);
}

TEST_CASE("Tasks can be retrieved",
          "[TaskRepository]") {

    TaskRepository repository(":memory:");

    repository.addTask("Learn vectors");
    repository.addTask("Learn iterators");

    const auto tasks = repository.getAllTasks();

    REQUIRE(tasks.size() == 2);

    CHECK(tasks[0].title == "Learn vectors");
    CHECK(tasks[1].title == "Learn iterators");
}

TEST_CASE("A task can be marked completed",
          "[TaskRepository]") {

    TaskRepository repository(":memory:");

    const Task task = repository.addTask("Learn SQLite");

    const auto updated =
        repository.updateTask(task.id, true);

    REQUIRE(updated.has_value());

    CHECK(updated->id == task.id);
    CHECK(updated->title == "Learn SQLite");
    CHECK(updated->completed == true);

    // Verify the database itself was updated
    const auto tasks = repository.getAllTasks();

    REQUIRE(tasks.size() == 1);
    CHECK(tasks[0].completed == true);
}

TEST_CASE("Updating a nonexistent task returns nullopt",
          "[TaskRepository]") {

    TaskRepository repository(":memory:");

    const auto result =
        repository.updateTask(999, true);

    REQUIRE_FALSE(result.has_value());
}

TEST_CASE("A task can be deleted",
          "[TaskRepository]") {

    TaskRepository repository(":memory:");

    const Task task = repository.addTask("Delete me");

    const bool deleted =
        repository.deleteTask(task.id);

    REQUIRE(deleted == true);

    const auto tasks = repository.getAllTasks();

    CHECK(tasks.empty());
}

TEST_CASE("Deleting a nonexistent task returns false",
          "[TaskRepository]") {

    TaskRepository repository(":memory:");

    const bool deleted =
        repository.deleteTask(999);

    REQUIRE(deleted == false);
}


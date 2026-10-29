#include <crow.h>
#include "GreetingService.h"
#include "TaskRepository.h"

int main()
{
    crow::SimpleApp app;
    GreetingService greetingService;
    TaskRepository taskRepository;

    CROW_ROUTE(app, "/api/hello")([&greetingService](const crow::request &req)
                                  {
        // retrieve optional parameter
        const char* nameParam = req.url_params.get("name");
        
        // Convert to C++ string
        std::string name = nameParam ? nameParam : "World";

        // Generate the greeting
        std::string message = greetingService.greet(name);

        // Construct JSON response
        crow::json::wvalue response;
        response["message"] = message;
        return response; });

    CROW_ROUTE(app, "/api/tasks")
        .methods(crow::HTTPMethod::GET)([&taskRepository]()
                                        {

        const auto tasks = taskRepository.getAllTasks();

        std::vector<crow::json::wvalue> jsonTasks;

        for (const auto& task : tasks) {

            crow::json::wvalue item;

            item["id"] = task.id;
            item["title"] = task.title;
            item["completed"] = task.completed;

            jsonTasks.push_back(std::move(item));
        }

        crow::json::wvalue response;
        response["tasks"] = std::move(jsonTasks);

        return response; });

    CROW_ROUTE(app, "/api/tasks")
        .methods(crow::HTTPMethod::POST)([&taskRepository](const crow::request &req)
                                         {

        auto body = crow::json::load(req.body);

        if (!body ||
            !body.has("title") ||
            body["title"].t() != crow::json::type::String) {

            return crow::response(
                400, "A string title is required"
            );
        }

        std::string title = body["title"].s();

        if (title.empty() || title.size() > 200) {
            return crow::response(
                400, "Title must be between 1 and 200 bytes"
            );
        }

        Task task = taskRepository.addTask(title);

        crow::json::wvalue response;

        response["id"] = task.id;
        response["title"] = task.title;
        response["completed"] = task.completed;

        crow::response httpResponse(
            201, response.dump()
        );

        httpResponse.set_header(
            "Content-Type", "application/json"
        );

        return httpResponse; });

    CROW_ROUTE(app, "/api/tasks/<int>")
        .methods(crow::HTTPMethod::PATCH)([&taskRepository](const crow::request &req, int id)
                                          {

        // Validate the task ID
        if (id <= 0) {
            return crow::response(400, "Invalid task ID");
        }

        // Parse the JSON request body
        auto body = crow::json::load(req.body);

        if (!body ||
            body.t() != crow::json::type::Object ||
            !body.has("completed")) {

            return crow::response(
                400, "A boolean 'completed' field is required"
            );
        }

        // Ensure the field contains a JSON boolean
        const auto valueType = body["completed"].t();

        if (valueType != crow::json::type::True &&
            valueType != crow::json::type::False) {

            return crow::response(
                400, "'completed' must be true or false"
            );
        }

        const bool completed = body["completed"].b();

        // Update the task
        std::optional<Task> updated =
            taskRepository.updateTask(id, completed);

        if (!updated) {
            return crow::response(404, "Task not found");
        }

        // Construct the JSON response
        crow::json::wvalue response;

        response["id"] = updated->id;
        response["title"] = updated->title;
        response["completed"] = updated->completed;

        crow::response httpResponse(
            200, response.dump()
        );

        httpResponse.set_header(
            "Content-Type", "application/json"
        );


        return httpResponse; });

    CROW_ROUTE(app, "/api/tasks/<int>")
        .methods(crow::HTTPMethod::DELETE)([&taskRepository](int id)
                                           {

        // Validate the task ID
        if (id <= 0) {
            return crow::response(
                400, "Invalid task ID"
            );
        }

        // Attempt to delete the task
        const bool deleted = taskRepository.deleteTask(id);

        if (!deleted) {
            return crow::response(
                404, "Task not found"
            );
        }

        // Successful deletion: no response body
        return crow::response(204); });

    app.bindaddr("0.0.0.0").port(18080).multithreaded().run();
}

#include "ApiClient.h"

#include <Wt/Json/Object.h>
#include <Wt/Json/Parser.h>
#include <Wt/Utils.h>
#include <Wt/Json/Array.h>
#include <Wt/Json/Serializer.h>
#include <Wt/Http/Message.h>
#include <Wt/WString.h>

#include <chrono>
#include <memory>
#include <stdexcept>
#include <utility>

#include <boost/system/error_code.hpp>

ApiClient::ApiClient(std::string baseUrl)
    : baseUrl_(std::move(baseUrl))
{

    client_ = addChild(std::make_unique<Wt::Http::Client>());
    taskClient_ = addChild(std::make_unique<Wt::Http::Client>());
    createClient_ = addChild(std::make_unique<Wt::Http::Client>());

    client_->setTimeout(std::chrono::seconds(5));
    taskClient_->setTimeout(std::chrono::seconds{5});
    createClient_->setMaximumResponseSize(16 * 1024);

    client_->setMaximumResponseSize(16 * 1024);
    taskClient_->setMaximumResponseSize(256 * 1024);
    createClient_->setTimeout(std::chrono::seconds{5});

    client_->done().connect(
        [this](boost::system::error_code error, const Wt::Http::Message &response)
        {
            handleResponse(
                static_cast<bool>(error),
                response.status(),
                response.body(),
                error.message());
        });

    taskClient_->done().connect(
        [this](boost::system::error_code error, const Wt::Http::Message &response)
        {
            handleTasksResponse(
                static_cast<bool>(error),
                response.status(),
                response.body(),
                error.message());
        });

    createClient_->done().connect(
        [this](boost::system::error_code error,
               const Wt::Http::Message &response)
        {
            handleCreateResponse(
                static_cast<bool>(error),
                response.status(),
                error.message());
        });
}

bool ApiClient::getGreeting(const std::string &name, GreetingCallback callback)
{
    // one request at a time
    if (busy_)
    {
        return false;
    }

    const std::string url = baseUrl_ + "/api/hello?name=" + Wt::Utils::urlEncode(name);

    busy_ = true;

    callback_ = std::move(callback);

    try
    {
        if (!client_->get(url))
        {
            busy_ = false;
            callback_ = {};
            return false;
        }
    }
    catch (...)
    {
        busy_ = false;
        callback_ = {};
        throw;
    }

    return true;
}

void ApiClient::handleResponse(
    bool failed,
    int status,
    const std::string &body,
    const std::string &error)
{
    std::string message;
    std::string failure;

    try
    {
        if (failed)
        {
            throw std::runtime_error(error);
        }

        if (status != 200)
        {
            throw std::runtime_error("Crow returned HTTP " + std::to_string(status));
        }

        Wt::Json::Object json;
        Wt::Json::parse(body, json);

        if (json.type("message") != Wt::Json::Type::String)
        {
            throw std::runtime_error("Invalid greeting response");
        }

        message = static_cast<std::string>(json.get("message"));
    }
    catch (const std::exception &e)
    {
        failure = e.what();
    }

    // Mark complete
    busy_ = false;

    auto callback = std::move(callback_);

    if (callback)
    {
        callback(message, failure);
    }
}

bool ApiClient::getTasks(TasksCallback callback)
{

    if (tasksBusy_)
    {
        return false;
    }

    tasksBusy_ = true;
    tasksCallback_ = std::move(callback);

    try
    {
        if (!taskClient_->get(baseUrl_ + "/api/tasks"))
        {
            tasksBusy_ = false;
            tasksCallback_ = {};
            return false;
        }
    }
    catch (...)
    {
        tasksBusy_ = false;
        tasksCallback_ = {};
        throw;
    }

    return true;
}

void ApiClient::handleTasksResponse(
    bool failed,
    int status,
    const std::string &body,
    const std::string &error)
{
    std::vector<Task> tasks;
    std::string failure;

    try
    {
        if (failed)
        {
            throw std::runtime_error(error);
        }

        if (status != 200)
        {
            throw std::runtime_error(
                "Crow returned HTTP " + std::to_string(status));
        }

        // Parse the JSON returned by Crow
        Wt::Json::Object json;
        Wt::Json::parse(body, json);

        if (json.type("tasks") != Wt::Json::Type::Array)
        {
            throw std::runtime_error("Missing tasks array");
        }

        // Extract the array
        Wt::Json::Array jsonTasks =
            static_cast<Wt::Json::Array>(json.get("tasks"));

        for (const auto &value : jsonTasks)
        {

            if (value.type() != Wt::Json::Type::Object)
            {
                throw std::runtime_error("Invalid task object");
            }

            Wt::Json::Object item =
                static_cast<Wt::Json::Object>(value);

            if (item.type("id") != Wt::Json::Type::Number ||
                item.type("title") != Wt::Json::Type::String ||
                item.type("completed") != Wt::Json::Type::Bool)
            {

                throw std::runtime_error("Invalid task fields");
            }

            Task task{
                static_cast<int>(item.get("id")),
                static_cast<std::string>(item.get("title")),
                static_cast<bool>(item.get("completed"))};

            tasks.push_back(std::move(task));
        }
    }
    catch (const std::exception &e)
    {
        failure = e.what();
    }

    tasksBusy_ = false;

    auto callback = std::move(tasksCallback_);
    tasksCallback_ = {};

    if (callback)
    {
        callback(tasks, failure);
    }
}

bool ApiClient::createTask(
    const std::string &title,
    CreateTaskCallback callback)
{
    if (createBusy_)
    {
        return false;
    }

    if (title.empty() || title.size() > 200)
    {
        throw std::invalid_argument(
            "Task title must contain 1 to 200 bytes");
    }

    // Construct the JSON request
    Wt::Json::Object json;
    json["title"] = Wt::WString::fromUTF8(title);

    const std::string body = Wt::Json::serialize(json);

    // Construct the HTTP message
    Wt::Http::Message request;

    request.setHeader(
        "Content-Type",
        "application/json");

    request.addBodyText(body);

    createBusy_ = true;
    createCallback_ = std::move(callback);

    try
    {
        const bool started = createClient_->post(
            baseUrl_ + "/api/tasks",
            request);

        if (!started)
        {
            createBusy_ = false;
            createCallback_ = {};
            return false;
        }
    }
    catch (...)
    {
        createBusy_ = false;
        createCallback_ = {};
        throw;
    }

    return true;
}

void ApiClient::handleCreateResponse(
    bool failed,
    int status,
    const std::string &error)
{
    std::string failure;

    if (failed)
    {
        failure = error.empty()
                      ? "Network request failed"
                      : error;
    }
    else if (status != 201)
    {
        failure = "Crow returned HTTP " +
                  std::to_string(status);
    }

    createBusy_ = false;

    auto callback = std::move(createCallback_);
    createCallback_ = {};

    if (callback)
    {
        callback(failure);
    }
}
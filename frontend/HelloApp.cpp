#include "HelloApp.h"

#include <Wt/WContainerWidget.h>
#include <Wt/WString.h>
#include <Wt/WBreak.h>
#include <Wt/WTableCell.h>

#include <memory>
#include <exception>
#include <string>

HelloApp::HelloApp(const Wt::WEnvironment &env) : Wt::WApplication(env)
{
    setTitle("Crow + Wt C++ demo");

    // allow completed async requests to update the UI
    enableUpdates();

    // Create the API client
    apiClient_ = addChild(std::make_unique<ApiClient>("http://localhost:18080"));

    root()->addWidget(std::make_unique<Wt::WText>("Crow + Wt: async C++ demo"));

    root()->addWidget(std::make_unique<Wt::WText>(" — "));

    nameInput_ = root()->addWidget(
        std::make_unique<Wt::WLineEdit>());
    nameInput_->setPlaceholderText("Enter your name");

    button_ = root()->addWidget(std::make_unique<Wt::WPushButton>("Call Crow API"));

    root()->addWidget(std::make_unique<Wt::WText>(" — "));

    resultText_ = root()->addWidget(std::make_unique<Wt::WText>("Click the button to call the backend."));

    resultText_->setTextFormat(Wt::TextFormat::Plain);

    // Connect the click event to our member function
    button_->clicked().connect(this, &HelloApp::handleGreeting);

    // Task section
    // Task management section
    root()->addWidget(
        std::make_unique<Wt::WBreak>());

    root()->addWidget(
        std::make_unique<Wt::WText>(
            "<h3>Task Manager</h3>"));

    // Task title input
    taskInput_ = root()->addWidget(
        std::make_unique<Wt::WLineEdit>());

    taskInput_->setPlaceholderText("Enter a new task");

    // Add Task button
    addTaskButton_ = root()->addWidget(
        std::make_unique<Wt::WPushButton>("Add Task"));

    // Connect the button to the handler
    addTaskButton_->clicked().connect(
        this, &HelloApp::addTask);

    refreshButton_ = root()->addWidget(
        std::make_unique<Wt::WPushButton>("Refresh Tasks"));

    taskStatus_ = root()->addWidget(
        std::make_unique<Wt::WText>("Click Refresh Tasks to load."));

    taskStatus_->setTextFormat(Wt::TextFormat::Plain);

    taskTable_ = root()->addWidget(
        std::make_unique<Wt::WTable>());

    // Connect the Refresh button
    refreshButton_->clicked().connect(
        this, &HelloApp::refreshTasks);
}

void HelloApp::handleGreeting()
{
    const std::string name = nameInput_->text().toUTF8();

    // Disable button until request complete
    button_->setDisabled(true);
    resultText_->setText("Loading...");

    try
    {
        const bool started = apiClient_->getGreeting(name, [this](const std::string &message, const std::string &error)
                                                     {
            if(!error.empty()) {
                resultText_->setText(Wt::WString::fromUTF8("Request failed: " + error));
            } else {
                resultText_->setText(Wt::WString::fromUTF8(message));
            }

            // Renable button
            button_->setDisabled(false);

            // push result
            triggerUpdate(); });

        if (!started)
        {
            button_->setDisabled(false);
            resultText_->setText("Could not start HTTP request.");
        }
    }
    catch (const std::exception &e)
    {
        resultText_->setText(
            Wt::WString::fromUTF8(std::string("Request failed: ") + e.what()));
    }
}

void HelloApp::refreshTasks()
{

    refreshButton_->setDisabled(true);
    taskStatus_->setText("Loading tasks...");

    try
    {
        bool started = apiClient_->getTasks(
            [this](const std::vector<Task> &tasks,
                   const std::string &error)
            {
                if (!error.empty())
                {
                    taskStatus_->setText(
                        Wt::WString::fromUTF8(
                            "Error: " + error));
                }
                else
                {
                    displayTasks(tasks);
                    taskStatus_->setText(
                        "Loaded " +
                        std::to_string(tasks.size()) +
                        " task(s).");
                }

                refreshButton_->setDisabled(false);

                triggerUpdate();
            });

        if (!started)
        {
            refreshButton_->setDisabled(false);
            taskStatus_->setText("Could not start request.");
        }
    }
    catch (const std::exception &e)
    {
        refreshButton_->setDisabled(false);
        taskStatus_->setText(
            Wt::WString::fromUTF8(e.what()));
    }
}

void HelloApp::displayTasks(
    const std::vector<Task> &tasks)
{
    // Remove previous rows
    taskTable_->clear();

    // Column headers
    taskTable_->elementAt(0, 0)->addWidget(
        std::make_unique<Wt::WText>("ID"));

    taskTable_->elementAt(0, 1)->addWidget(
        std::make_unique<Wt::WText>("Title"));

    taskTable_->elementAt(0, 2)->addWidget(
        std::make_unique<Wt::WText>("Status"));

    taskTable_->setHeaderCount(1);

    int row = 1;

    for (const Task &task : tasks)
    {

        taskTable_->elementAt(row, 0)->addWidget(
            std::make_unique<Wt::WText>(
                std::to_string(task.id)));

        auto titleWidget = taskTable_->elementAt(row, 1)
                               ->addWidget(
                                   std::make_unique<Wt::WText>(
                                       Wt::WString::fromUTF8(task.title)));

        titleWidget->setTextFormat(Wt::TextFormat::Plain);

        taskTable_->elementAt(row, 2)->addWidget(
            std::make_unique<Wt::WText>(
                task.completed ? "Completed" : "Pending"));

        ++row;
    }
}

void HelloApp::addTask()
{

    // Read the title from the UI
    const std::string title =
        taskInput_->text().toUTF8();

    if (title.empty() || title.size() > 200)
    {
        taskStatus_->setText(
            "Task title must contain 1 to 200 bytes.");
        return;
    }

    // Prevent duplicate submissions
    addTaskButton_->setDisabled(true);
    taskStatus_->setText("Creating task...");

    try
    {
        const bool started = apiClient_->createTask(
            title,

            [this](const std::string &error)
            {
                addTaskButton_->setDisabled(false);

                if (!error.empty())
                {
                    taskStatus_->setText(
                        Wt::WString::fromUTF8(
                            "Failed to create task: " + error));
                }
                else
                {
                    // Clear input after successful creation
                    taskInput_->setText("");

                    taskStatus_->setText("Task created.");

                    // Retrieve the updated task list
                    refreshTasks();
                }

                triggerUpdate();
            });

        if (!started)
        {
            addTaskButton_->setDisabled(false);
            taskStatus_->setText(
                "Could not start task creation request.");
        }
    }
    catch (const std::exception &e)
    {
        addTaskButton_->setDisabled(false);
        taskStatus_->setText(
            Wt::WString::fromUTF8(
                std::string("Error: ") + e.what()));
    }
}

#pragma once

#include "ApiClient.h"

#include <Wt/WApplication.h>
#include <Wt/WEnvironment.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>
#include <Wt/WTable.h>

#include <vector>
#include <string>

class HelloApp : public Wt::WApplication
{
public:
    explicit HelloApp(const Wt::WEnvironment &env);

private:
    void handleGreeting();
    void refreshTasks();
    void displayTasks(const std::vector<Task> &tasks);
    void addTask();

    ApiClient *apiClient_ = nullptr;

    Wt::WLineEdit *nameInput_ = nullptr;
    Wt::WPushButton *button_ = nullptr;
    Wt::WText *resultText_ = nullptr;
    Wt::WPushButton *refreshButton_ = nullptr;
    Wt::WTable *taskTable_ = nullptr;
    Wt::WText *taskStatus_ = nullptr;
    Wt::WLineEdit *taskInput_ = nullptr;
    Wt::WPushButton *addTaskButton_ = nullptr;
};
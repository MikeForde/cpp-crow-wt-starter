#pragma once

#include "ApiClient.h"

#include <Wt/WApplication.h>
#include <Wt/WEnvironment.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>

#include <string>

class HelloApp: public Wt::WApplication 
{
    public:
        explicit HelloApp(const Wt::WEnvironment& env);

    private:
        void handleGreeting();

        ApiClient* apiClient_ = nullptr;

        Wt::WLineEdit* nameInput_ = nullptr;
        Wt::WPushButton* button_ = nullptr;
        Wt::WText* resultText_ = nullptr;
};
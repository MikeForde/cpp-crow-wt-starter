#include "HelloApp.h"

#include <Wt/WContainerWidget.h>
#include <Wt/WString.h>

#include <memory>
#include <exception>
#include <string>

HelloApp::HelloApp(const Wt::WEnvironment &env) : Wt::WApplication(env)
{
    setTitle("Crow + Wt C++ demo");

    // allow completed async requests to update the UI
    enableUpdates();

    // Create the API client
    apiClient_= addChild(std::make_unique<ApiClient>("http://localhost:18080"));

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
}

void HelloApp::handleGreeting()
{
    const std::string name = nameInput_->text().toUTF8();

    // Disable button until request complete
    button_->setDisabled(true);
    resultText_->setText("Loading...");

    try
    {
        const bool started = apiClient_->getGreeting(name, [this](const std::string& message, const std::string& error) {
            if(!error.empty()) {
                resultText_->setText(Wt::WString::fromUTF8("Request failed: " + error));
            } else {
                resultText_->setText(Wt::WString::fromUTF8(message));
            }

            // Renable button
            button_->setDisabled(false);

            // push result
            triggerUpdate();
        });

        if(!started) {
            button_->setDisabled(false);
            resultText_->setText("Could not start HTTP request.");
        }
    }
    catch (const std::exception &e)
    {
        resultText_->setText(
            Wt::WString::fromUTF8(std::string("Request failed: ") + e.what())
        );
    }
}

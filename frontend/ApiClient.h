#pragma once

#include <Wt/WObject.h>
#include <Wt/Http/Client.h>

#include <functional>
#include <string>

class ApiClient : public Wt::WObject
{
public:
    using GreetingCallback = std::function<void(const std::string& message, const std::string& error)>;

    explicit ApiClient(std::string baseUrl);

    bool getGreeting(const std::string& name, GreetingCallback callback);

private:
    std::string baseUrl_;

    Wt::Http::Client* client_ = nullptr;

    GreetingCallback callback_;

    bool busy_ = false;

    void handleResponse(
        bool failed,
        int status,
        const std::string& body,
        const std::string& error
    );
};
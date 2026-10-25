#include "ApiClient.h"

#include <Wt/Json/Object.h>
#include <Wt/Json/Parser.h>
#include <Wt/Utils.h>

#include <chrono>
#include <memory>
#include <stdexcept>
#include <utility>

#include <boost/system/error_code.hpp>

ApiClient::ApiClient(std::string baseUrl)
    : baseUrl_(std::move(baseUrl))
{

    client_ = addChild(std::make_unique<Wt::Http::Client>());

    client_->setTimeout(std::chrono::seconds(5));

    client_->setMaximumResponseSize(16 * 1024);

    client_->done().connect(
        [this](boost::system::error_code error, const Wt::Http::Message &response)
        {
            handleResponse(
                static_cast<bool>(error),
                response.status(),
                response.body(),
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
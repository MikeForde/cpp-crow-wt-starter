#include <crow.h>
#include "GreetingService.h"

int main() {
    crow::SimpleApp app;
    GreetingService greetingService;


    CROW_ROUTE(app, "/api/hello")([&greetingService](const crow::request& req) {
        // retrieve optional parameter
        const char* nameParam = req.url_params.get("name");
        
        // Convert to C++ string
        std::string name = nameParam ? nameParam : "World";

        // Generate the greeting
        std::string message = greetingService.greet(name);

        // Construct JSON response
        crow::json::wvalue response;
        response["message"] = message;
        return response;
    });

    app.bindaddr("0.0.0.0").port(18080).multithreaded().run();
}

#include "GreetingService.h"

std::string GreetingService::greet(const std::string& name) const {
    if (name.empty()) {
        return "Hello, World!";
    }


    return "Hello, " + name + "!";
}
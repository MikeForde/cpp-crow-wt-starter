#pragma once

#include <string>

class GreetingService {
    public:
        std::string greet(const std::string& name) const;
};
#include "HelloApp.h"

#include <Wt/WApplication.h>

#include <memory>

int main(int argc, char** argv) {
    return Wt::WRun(
        argc,
        argv,
        [](const Wt::WEnvironment& env) {
            return std::make_unique<HelloApp>(env);
        }
    );
}
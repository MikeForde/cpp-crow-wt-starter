cmake_minimum_required(VERSION 3.20)
project(CrowWtStarter LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(CTest)
include(FetchContent)
set(CROW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CROW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(Crow
    GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
    GIT_TAG v1.3.1
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(Crow)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.15.2
)

FetchContent_MakeAvailable(Catch2)

find_package(Wt REQUIRED COMPONENTS Wt HTTP)
find_package(SQLite3 REQUIRED)

add_executable(crow_api 
    backend/main.cpp
    backend/GreetingService.cpp
    backend/TaskRepository.cpp
    backend/Database.cpp
    backend/Statement.cpp
    backend/Transaction.cpp
)

target_link_libraries(crow_api PRIVATE
    Crow::Crow
    SQLite::SQLite3
)

add_executable(wt_ui 
    frontend/main.cpp
    frontend/ApiClient.cpp
    frontend/HelloApp.cpp
)
target_link_libraries(wt_ui PRIVATE Wt::Wt Wt::HTTP)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

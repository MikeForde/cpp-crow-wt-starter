add_executable(task_tests
    test_task_repository.cpp

    ../backend/TaskRepository.cpp
    ../backend/Database.cpp
    ../backend/Statement.cpp
    ../backend/Transaction.cpp
)

target_include_directories(task_tests PRIVATE
    ../backend
)

target_link_libraries(task_tests PRIVATE
    SQLite::SQLite3
    Catch2::Catch2WithMain
)

list(APPEND CMAKE_MODULE_PATH
    ${catch2_SOURCE_DIR}/extras
)

include(Catch)

catch_discover_tests(task_tests)


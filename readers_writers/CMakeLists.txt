cmake_minimum_required(VERSION 3.5)

project(readers_writers LANGUAGES CXX)

find_package(Threads REQUIRED)

set(THREADS_PREFER_PTHREAD_FLAG ON)

set(PROJECT_SOURCES
        src/main.cpp
        )
add_executable(${PROJECT_NAME}
            ${PROJECT_SOURCES}
            )

target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)

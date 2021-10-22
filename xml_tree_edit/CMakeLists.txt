cmake_minimum_required(VERSION 3.5)

project(xml_tree_edit LANGUAGES CXX)

set(PROJECT_SOURCES
        src/main.cpp
        )
		
add_executable(${PROJECT_NAME}
            ${PROJECT_SOURCES}
            )

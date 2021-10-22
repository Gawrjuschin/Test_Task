cmake_minimum_required(VERSION 3.5)

project(Test_Task)

option(BUILD_READERS_WRITERS "Enable building readers_writers" ON)
option(BUILD_XML_TREE_EDIT "Enable building xml_tree_edit" ON)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

if(BUILD_READERS_WRITERS)
    add_subdirectory(readers_writers)
endif()

if(BUILD_XML_TREE_EDIT)
    add_subdirectory(xml_tree_edit)
endif()

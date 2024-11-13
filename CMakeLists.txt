cmake_minimum_required(VERSION 3.15...3.31)

project(ECUtool VERSION 0.1.0
                DESCRIPTION "A GUI application for reverse-engineering ECUs"
                LANGUAGES CXX)

set(wxBUILD_SHARED OFF)

add_executable(ecutoolapp WIN32)

set_property(TARGET ecutoolapp PROPERTY CXX_STANDARD 20)
set_property(TARGET ecutoolapp PROPERTY CXX_STANDARD_REQUIRED TRUE)

add_subdirectory(ECUtool)
add_subdirectory(libs/wxWidgets)

target_link_libraries(ecutoolapp wx::core wx::base)

target_compile_definitions(ecutoolapp PRIVATE ECUtool_VERSION_STRING="${ECUtool_VERSION_MAJOR}.${ECUtool_VERSION_MINOR}.${ECUtool_VERSION_PATCH}")
cmake_minimum_required(VERSION 3.15...3.31)

project(ECUtool VERSION 0.1.0
                DESCRIPTION "A GUI application for reverse-engineering ECUs"
                LANGUAGES CXX)

set(wxBUILD_SHARED OFF)

add_executable(ecutool WIN32)

add_subdirectory(ECUtool)
add_subdirectory(libs/wxWidgets)

target_link_libraries(ecutool wx::net wx::core wx::base)
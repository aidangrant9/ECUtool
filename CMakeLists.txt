cmake_minimum_required(VERSION 3.23)

project(ECUtool VERSION 0.2.0
                DESCRIPTION "A GUI application for reverse-engineering ECUs"
                LANGUAGES CXX)

include(vcpkg/scripts/buildsystems/vcpkg.cmake)


set(CMAKE_AUTOMOC ON)
find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

qt_add_executable(ecutoolapp "ECUtool/main.cpp")

set_target_properties(ecutoolapp PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED TRUE
)

add_subdirectory(ECUtool)

target_link_libraries(ecutoolapp PRIVATE OneCore Qt6::Widgets)

target_compile_definitions(ecutoolapp PRIVATE ECUtool_VERSION_STRING="${ECUtool_VERSION_MAJOR}.${ECUtool_VERSION_MINOR}.${ECUtool_VERSION_PATCH}")

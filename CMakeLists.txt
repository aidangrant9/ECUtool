cmake_minimum_required(VERSION 3.23)

project(ECUtool VERSION 0.2.0
                DESCRIPTION "A GUI application for reverse-engineering ECUs"
                LANGUAGES CXX)

add_executable(ecutoolapp)

include(vcpkg/scripts/buildsystems/vcpkg.cmake)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

set_property(TARGET ecutoolapp PROPERTY CXX_STANDARD 20)
set_property(TARGET ecutoolapp PROPERTY CXX_STANDARD_REQUIRED TRUE)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(MSVC)
        target_link_options(ecutoolapp PRIVATE $<$<CONFIG:Debug>:/INCREMENTAL>)
        target_compile_options(ecutoolapp PRIVATE $<$<CONFIG:Debug>:/ZI>)
    else()
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
    endif()
endif()

add_subdirectory(ECUtool)

target_link_libraries(ecutoolapp PRIVATE OneCore Qt6::Widgets)

target_compile_definitions(ecutoolapp PRIVATE ECUtool_VERSION_STRING="${ECUtool_VERSION_MAJOR}.${ECUtool_VERSION_MINOR}.${ECUtool_VERSION_PATCH}")
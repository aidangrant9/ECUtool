cmake_minimum_required(VERSION 3.23)

project(ECUtool VERSION 0.1.0
                DESCRIPTION "A GUI application for reverse-engineering ECUs"
                LANGUAGES CXX)

set(wxBUILD_SHARED OFF)

add_executable(ecutoolapp WIN32)

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
add_subdirectory(libs/wxWidgets)

target_link_libraries(ecutoolapp PRIVATE OneCore wx::core wx::base)

target_compile_definitions(ecutoolapp PRIVATE ECUtool_VERSION_STRING="${ECUtool_VERSION_MAJOR}.${ECUtool_VERSION_MINOR}.${ECUtool_VERSION_PATCH}")
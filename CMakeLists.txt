cmake_minimum_required(VERSION 3.23)

project(ECUtool VERSION 1.0.0
                DESCRIPTION "A GUI application for reverse-engineering ECUs"
                LANGUAGES CXX)

set(CMAKE_OSX_ARCHITECTURES "arm64")

set(BUILD_SHARED_LIBS OFF)


include(vcpkg/scripts/buildsystems/vcpkg.cmake)


set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Core Svg)
find_package(nlohmann_json REQUIRED)
find_package(lua REQUIRED)

qt_standard_project_setup()

qt_add_executable(ecutoolapp "ECUtool/main.cpp")

set_target_properties(ecutoolapp PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED TRUE
)

if(WIN32)
    target_link_libraries(ecutoolapp PRIVATE OneCore)
endif()

add_subdirectory(ECUtool)
add_subdirectory(serial)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "AppleClang")
    target_compile_options(ecutoolapp PRIVATE -fexperimental-library)
endif()

target_link_libraries(ecutoolapp PRIVATE Qt6::Widgets Qt6::Core Qt6::Svg nlohmann_json::nlohmann_json lua serial)
target_include_directories(ecutoolapp PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/ecutoolapp_autogen/include ${CMAKE_SOURCE_DIR}/ECUtool/ui)
target_compile_definitions(ecutoolapp PRIVATE ECUtool_VERSION_STRING="${ECUtool_VERSION_MAJOR}.${ECUtool_VERSION_MINOR}.${ECUtool_VERSION_PATCH}")

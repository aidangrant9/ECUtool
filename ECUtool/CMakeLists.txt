target_sources(ecutoolapp PRIVATE "main.cpp")

target_include_directories(ecutoolapp PRIVATE communication ui)

add_subdirectory(ui)
add_subdirectory(communication)
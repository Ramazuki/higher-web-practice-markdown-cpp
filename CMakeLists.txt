cmake_minimum_required(VERSION 3.30)
project(MarkdownToHTML)

set(CMAKE_CXX_STANDARD 20)

set(SOURCES
        main.cpp
)

add_subdirectory(converter)
include_directories(converter)

add_executable(MarkdownToHTML main.cpp)

target_link_libraries(
        MarkdownToHTML
        PRIVATE
        MarkdownConverter
)

add_executable(MarkdownTests tests/test_main.cpp)

target_link_libraries(
        MarkdownTests
        PRIVATE
        MarkdownConverter
)

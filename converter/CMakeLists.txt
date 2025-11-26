
set(SOURCES
        inline_parser.cpp
        renderer.cpp
        scanner.cpp
        utils.cpp
)
set(HEADERS
        inline_parser.h
        renderer.h
        scanner.h
        utils.h
)

add_library(MarkdownConverter ${SOURCES})

set_target_properties(MarkdownConverter
        PROPERTIES
        PUBLIC_HEADER "${HEADERS}"
)

install(
        TARGETS MarkdownConverter
        PUBLIC_HEADER DESTINATION include
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
)
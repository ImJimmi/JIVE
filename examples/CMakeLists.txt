include(${PROJECT_SOURCE_DIR}/cmake/jive_add_example.cmake)

add_subdirectory(shared)

add_subdirectory(01-text)
add_subdirectory(02-layouts)
add_subdirectory(03-style-sheets)
add_subdirectory(04-transitions)
add_subdirectory(05-widgets)

add_subdirectory(gain-plugin)
add_subdirectory(jive-demo)
add_subdirectory(xml-views)

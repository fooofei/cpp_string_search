cmake_minimum_required (VERSION 2.6)

set(BINARY_NAME cpp_string_search)

project (${BINARY_NAME})


add_executable(${BINARY_NAME} test.cpp)


if (NOT TARGET string_search)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../string_search  build_string_search)
endif ()
target_link_libraries(${BINARY_NAME} string_search)

add_custom_command(
        TARGET ${BINARY_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${BINARY_NAME}> ../$<TARGET_FILE_NAME:${BINARY_NAME}>
)

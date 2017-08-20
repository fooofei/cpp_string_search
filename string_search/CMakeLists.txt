# This CMakeLists.txt is for encoding module.
# Other Module can use it by this:
#
# if (NOT TARGET encoding)
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../string_search  build_string_search)
# endif ()
# target_link_libraries(${PROJECT_NAME} string_search)
#
# add_subdirectory() is add this module for build.
# first argument is to tell the module(CMakeLists.txt) location,
# second argument is to tell where to build the module, make sure not to use
# the same directory with the current module.
#
# target_link_libraries() tell to link the module.
#
#

cmake_minimum_required(VERSION 2.8.14)
project(string_search)

if (APPLE)
    set(CMAKE_MACOSX_RPATH 0)
endif ()


# point to the who include this CMakeLists.txt
#message(STATUS "CMAKE_HOME_DIRECTORY=" ${CMAKE_HOME_DIRECTORY})

# point to this CMakeLists.txt
#message(STATUS "CMAKE_CURRENT_SOURCE_DIR=" ${CMAKE_CURRENT_SOURCE_DIR})

# point to this CMakeLists.txt
#message(STATUS "CMAKE_CURRENT_LIST_DIR=" ${CMAKE_CURRENT_LIST_DIR})

add_library(${PROJECT_NAME} STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/string_search.cpp)
target_sources(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wumanber_search.cpp
    )


# for others module include
target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

if (NOT WIN32)
    # fPIC for other library link
    target_compile_options(${PROJECT_NAME} PRIVATE -fPIC)
endif()

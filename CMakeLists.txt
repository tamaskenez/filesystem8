cmake_minimum_required(VERSION 3.1)
project(filesystem)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN)
set(CMAKE_DEBUG_POSTFIX _d)

include(CheckIncludeFile)
check_include_file(unistd.h BOOST_HAS_UNISTD_H)
if(BOOST_HAS_UNISTD_H)
    add_definitions(-DBOOST_HAS_UNISTD_H)
endif()

add_subdirectory(src)

if(NOT CMAKE_CROSSCOMPILING)
    enable_testing()
endif()

if(NOT DEFINED BUILD_TESTING OR BUILD_TESTING)
    add_subdirectory(test)
endif()

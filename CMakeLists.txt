cmake_minimum_required(VERSION 3.10)
project(pathfinding_visualizer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

find_package(Threads REQUIRED)

add_executable(server main.cpp)
target_link_libraries(server PRIVATE Threads::Threads)
target_include_directories(server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

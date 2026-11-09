cmake_minimum_required(VERSION 3.10)
project(task5)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(task5 main.cpp simulator.cpp my_scheduler.cpp)

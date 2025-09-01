
cmake_minimum_required(VERSION 3.10)

project(assesment LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

target_include_directories(assesment PUBLIC include)
add_executable(assesment main.cpp)
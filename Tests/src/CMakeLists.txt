cmake_minimum_required(VERSION 3.14)
project(Receiver)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(${PROJECT_NAME} Receiver.cpp)
add_executable(Sender Sender.cpp)

add_subdirectory(Test)


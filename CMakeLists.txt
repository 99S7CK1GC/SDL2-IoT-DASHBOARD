cmake_minimum_required(VERSION 3.16)

project(IoTDashboard LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)

file(GLOB_RECURSE SOURCES src/*.cpp)

add_executable(IoTDashboard ${SOURCES})

target_include_directories(IoTDashboard PRIVATE include)

target_link_libraries(IoTDashboard PRIVATE
    SDL2::SDL2
    SDL2_ttf::SDL2_ttf
)

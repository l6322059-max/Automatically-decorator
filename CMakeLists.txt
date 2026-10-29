cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(AutoDeco VERSION 1.0.0 LANGUAGES CXX)

# Configure this project with the Geode SDK's CMake toolchain.
# Geode's setup_geode_mod() generates the installable .geode package after building.
add_library(${PROJECT_NAME} SHARED
    src/main.cpp
)

setup_geode_mod(${PROJECT_NAME})

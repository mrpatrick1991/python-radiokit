cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(itm_bindings LANGUAGES CXX)

# Find the Pybind11 package
find_package(pybind11 REQUIRED)

# Add the ITM library with explicit source files
file(GLOB ITM_SOURCES "${CMAKE_SOURCE_DIR}/../../third_party/itm/src/*.cpp")
add_library(itm STATIC ${ITM_SOURCES})

# Specify include directories for the ITM library
target_include_directories(itm PUBLIC "${CMAKE_SOURCE_DIR}/../../third_party/itm/include")

target_compile_options(itm PRIVATE -fdeclspec -fms-extensions)

# Add the Pybind11 module with explicit path to itm_bindings.cpp
set(ITM_BINDINGS_SRC "${CMAKE_SOURCE_DIR}/itm_bindings.cpp")
pybind11_add_module(_itm_bindings ${ITM_BINDINGS_SRC})

# Link the ITM library to the Python extension
target_link_libraries(_itm_bindings PRIVATE itm)

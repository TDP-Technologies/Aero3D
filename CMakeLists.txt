cmake_minimum_required(VERSION 3.20)
project(Aero3D)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Build configurations
set(CMAKE_CONFIGURATION_TYPES Debug Release Dist CACHE STRING "" FORCE)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Vendor directory
set(DEPENDENCIES_DIR "${CMAKE_SOURCE_DIR}/vendor")

# Dependencies
add_subdirectory(${DEPENDENCIES_DIR}/SDL)
add_subdirectory(${DEPENDENCIES_DIR}/shaderc)

# Projects
add_subdirectory(Engine)
add_subdirectory(Sandbox)

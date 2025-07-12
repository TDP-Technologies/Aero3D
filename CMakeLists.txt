cmake_minimum_required(VERSION 3.20)
project(Aero3D VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Support multiple build configurations
set(CMAKE_CONFIGURATION_TYPES Debug Release Dist CACHE STRING "" FORCE)

# Global compile flags per build type
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DIST "-O3 -DNDEBUG -flto")

# Output directories for binaries and libraries
set(OUTPUT_ROOT ${CMAKE_BINARY_DIR})

foreach(OUTPUTCONFIG Debug Release Dist)
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG_UPPER)

    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} ${OUTPUT_ROOT}/bin/${OUTPUTCONFIG})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} ${OUTPUT_ROOT}/lib/${OUTPUTCONFIG})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} ${OUTPUT_ROOT}/bin/${OUTPUTCONFIG})
endforeach()

# Enable Position Independent Code for shared libraries
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Add external dependencies
add_subdirectory(vendor/SDL)

# Configure shaderc options before including it
set(SPIRV_TOOLS_SKIP_INSTALL ON)
add_subdirectory(vendor/shaderc)

add_subdirectory(vendor/volk)

# Configure Assimp options before including it
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT ON CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_DOCS OFF CACHE BOOL "" FORCE)
add_subdirectory(vendor/assimp)

# Add project subdirectories
add_subdirectory(Engine)
add_subdirectory(Sandbox)

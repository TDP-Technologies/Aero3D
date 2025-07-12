project(Engine LANGUAGES CXX)

# Collect common engine source files
file(GLOB_RECURSE ENGINE_SOURCES_COMMON
    src/Core/*.cpp
    src/Event/*.cpp
    src/Graphics/*.cpp
    src/Platform/Vulkan/*.cpp
    src/Resource/*.cpp
    src/IO/*.cpp
    src/Scene/*.cpp
    src/Systems/*.cpp
    src/Utils/*.cpp
    vendor/*.cpp
)

# Collect common engine header files
file(GLOB_RECURSE ENGINE_HEADERS_COMMON
    src/Core/*.h
    src/Event/*.h
    src/Graphics/*.h
    src/Platform/Vulkan/*.h
    src/Resource/*.h
    src/IO/*.h
    src/Scene/*.h
    src/Systems/*.h
    src/Utils/*.h
    vendor/*.h
)

# Platform-specific sources and headers
set(PLATFORM_SOURCES)
set(PLATFORM_HEADERS)

if(WIN32)
    file(GLOB_RECURSE PLATFORM_SOURCES src/Platform/Windows/*.cpp)
    file(GLOB_RECURSE PLATFORM_HEADERS src/Platform/Windows/*.h)
elseif(UNIX AND NOT APPLE)
    file(GLOB_RECURSE PLATFORM_SOURCES src/Platform/Linux/*.cpp)
    file(GLOB_RECURSE PLATFORM_HEADERS src/Platform/Linux/*.h)
endif()

# Merge all sources and headers
set(ENGINE_SOURCES ${ENGINE_SOURCES_COMMON} ${PLATFORM_SOURCES})
set(ENGINE_HEADERS ${ENGINE_HEADERS_COMMON} ${PLATFORM_HEADERS})

# Find required packages
find_package(Vulkan REQUIRED)
find_package(ZLIB REQUIRED)

# Create shared library target for the engine
add_library(Engine SHARED ${ENGINE_SOURCES} ${ENGINE_HEADERS})

# Define platform-specific Vulkan compile definitions
if(WIN32)
    target_compile_definitions(Engine PRIVATE VK_USE_PLATFORM_WIN32_KHR)
elseif(UNIX AND NOT APPLE)
    target_compile_definitions(Engine PRIVATE VK_USE_PLATFORM_XLIB_KHR)
endif()

# Include directories for dependencies and engine sources
target_include_directories(Engine PRIVATE
    ${CMAKE_SOURCE_DIR}/vendor/glm
    ${CMAKE_SOURCE_DIR}/vendor/shaderc/libshaderc/include
    ${CMAKE_SOURCE_DIR}/vendor/volk
    ${CMAKE_SOURCE_DIR}/vendor/assimp/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor
)

# Link libraries required by the engine
target_link_libraries(Engine PRIVATE
    ${Vulkan_LIBRARIES}
    SDL3::SDL3
    shaderc_combined
    volk
    assimp
    ZLIB::ZLIB
)

# Define compile-time definitions based on build configuration
target_compile_definitions(Engine PRIVATE
    $<$<CONFIG:Debug>:A3D_DEBUG>
    $<$<CONFIG:Release>:A3D_RELEASE>
    $<$<CONFIG:Dist>:A3D_DIST>
)

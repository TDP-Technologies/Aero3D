project(Engine)

# Common sources
file(GLOB_RECURSE ENGINE_SOURCES_COMMON
    src/Core/*.cpp
    src/Event/*.cpp
    src/Graphics/*.cpp
    src/Platform/Vulkan/*.cpp
    src/IO/*.cpp
    src/Scene/*.cpp
    src/Utils/*.cpp
    vendor/*.cpp
)

file(GLOB_RECURSE ENGINE_HEADERS_COMMON
    src/Core/*.h
    src/Event/*.h
    src/Graphics/*.h
    src/Platform/Vulkan/*.h
    src/IO/*.h
    src/Scene/*.h
    src/Utils/*.h
    vendor/*.h
)

set(PLATFORM_SOURCES)
set(PLATFORM_HEADERS)

# Platform dependent sources
if(WIN32)
    file(GLOB_RECURSE PLATFORM_SOURCES src/Platform/Windows/*.cpp)
    file(GLOB_RECURSE PLATFORM_HEADERS src/Platform/Windows/*.h)
elseif(UNIX)
    file(GLOB_RECURSE PLATFORM_SOURCES src/Platform/Linux/*.cpp)
    file(GLOB_RECURSE PLATFORM_HEADERS src/Platform/Linux/*.h)
endif()

# Merge all sources
set(ENGINE_SOURCES ${ENGINE_SOURCES_COMMON} ${PLATFORM_SOURCES})
set(ENGINE_HEADERS ${ENGINE_HEADERS_COMMON} ${PLATFORM_HEADERS})

# Vulkan dependency
find_package(Vulkan REQUIRED)

add_library(Engine SHARED ${ENGINE_SOURCES} ${ENGINE_HEADERS})

target_include_directories(Engine PRIVATE ${Vulkan_INCLUDE_DIR})
target_include_directories(Engine PRIVATE ${DEPENDENCIES_DIR}/glm)
target_include_directories(Engine PRIVATE ${DEPENDENCIES_DIR}/shaderc/libshaderc/include)
target_include_directories(Engine PRIVATE ${DEPENDENCIES_DIR}/volk)
target_include_directories(Engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(Engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/vendor)

# Projects dependencies
#target_link_libraries(Engine PRIVATE ${Vulkan_LIBRARIES})
target_link_libraries(Engine PRIVATE SDL3::SDL3)
target_link_libraries(Engine PRIVATE shaderc_combined)
target_link_libraries(Engine PRIVATE volk)

# Configurations
target_compile_definitions(Engine PRIVATE
    $<$<CONFIG:Debug>:A3D_DEBUG>
    $<$<CONFIG:Release>:A3D_RELEASE>
    $<$<CONFIG:Dist>:A3D_DIST>
)

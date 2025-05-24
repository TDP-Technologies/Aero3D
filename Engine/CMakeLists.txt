project(Engine)

file(GLOB_RECURSE ENGINE_SOURCES
    src/Core/*.cpp
    src/Event/*.cpp
    src/Graphics/*.cpp
    src/Platform/Vulkan/*.cpp
    src/Platform/Linux/*.cpp
    src/IO/*.cpp
    src/Scene/*.cpp
    src/Utils/*.cpp
    vendor/*.cpp
)

file(GLOB_RECURSE ENGINE_HEADERS
    src/Core/*.h
    src/Event/*.h
    src/Graphics/*.h
    src/Platform/Vulkan/*.h
    src/Platform/Linux/*.h
    src/IO/*.h
    src/Scene/*.h
    src/Utils/*.h
    vendor/*.h
)

find_package(Vulkan REQUIRED)

add_library(Engine SHARED ${ENGINE_SOURCES} ${ENGINE_HEADERS})

target_include_directories(Engine PRIVATE ${Vulkan_INCLUDE_DIR})
target_include_directories(Engine PRIVATE ${DEPENDENCIES_DIR}/glm)
target_include_directories(Engine PRIVATE ${DEPENDENCIES_DIR}/shaderc/libshaderc/include)
target_include_directories(Engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(Engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/vendor)

target_link_libraries(Engine PRIVATE ${Vulkan_LIBRARIES})
target_link_libraries(Engine PRIVATE SDL3::SDL3)
target_link_libraries(Engine PRIVATE shaderc_combined)

# Configurations
target_compile_definitions(Engine PRIVATE
    $<$<CONFIG:Debug>:A3D_DEBUG>
    $<$<CONFIG:Release>:A3D_RELEASE>
    $<$<CONFIG:Dist>:A3D_DIST>
)

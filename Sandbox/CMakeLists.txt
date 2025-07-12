project(Sandbox)

file(GLOB_RECURSE SANDBOX_SOURCES
    src/*.cpp
    src/*.h
)

add_executable(Sandbox ${SANDBOX_SOURCES})

target_include_directories(Sandbox PRIVATE
    src
    ${CMAKE_SOURCE_DIR}/Engine/src
    ${DEPENDENCIES_DIR}/glm
)

target_link_libraries(Sandbox PRIVATE Engine)

# Configurations
target_compile_definitions(Sandbox PRIVATE
    $<$<CONFIG:Debug>:A3D_DEBUG>
    $<$<CONFIG:Release>:A3D_RELEASE>
    $<$<CONFIG:Dist>:A3D_DIST>
)

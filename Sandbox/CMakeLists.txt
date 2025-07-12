project(Sandbox LANGUAGES CXX)

# Collect sandbox source files
file(GLOB_RECURSE SANDBOX_SOURCES
    src/*.cpp
    src/*.h
)

# Create executable target for Sandbox
add_executable(Sandbox ${SANDBOX_SOURCES})

# Include directories for Sandbox and Engine
target_include_directories(Sandbox PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/Engine/src
    ${CMAKE_SOURCE_DIR}/vendor/glm
)

# Link Sandbox against Engine library
target_link_libraries(Sandbox PRIVATE Engine)

# Define compile-time definitions based on build configuration
target_compile_definitions(Sandbox PRIVATE
    $<$<CONFIG:Debug>:A3D_DEBUG>
    $<$<CONFIG:Release>:A3D_RELEASE>
    $<$<CONFIG:Dist>:A3D_DIST>
)

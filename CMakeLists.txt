cmake_minimum_required(VERSION 3.24)
project(Projet)

set(CMAKE_CXX_STANDARD 14)

# Find OpenGL package
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

# Include directories for external libraries
include_directories(${PROJECT_SOURCE_DIR}/libs/glfw/include)
include_directories(${PROJECT_SOURCE_DIR}/libs/glew/include)
include_directories(${PROJECT_SOURCE_DIR}/libs/glm)
include_directories(${PROJECT_SOURCE_DIR}/common)

# Link directories for external libraries
link_directories(${PROJECT_SOURCE_DIR}/libs/glfw/lib-vc2022)
link_directories(${PROJECT_SOURCE_DIR}/libs/glew/lib/Release/x64)

# Find glm package
set(glm_DIR ${PROJECT_SOURCE_DIR}/libs/glm/cmake/glm)
find_package(glm REQUIRED)

# Source files
set(SOURCES
    ${PROJECT_SOURCE_DIR}/Projet/main.cpp
    ${PROJECT_SOURCE_DIR}/common/GLShader.cpp
)

# Add executable
add_executable(Projet ${SOURCES})

# Link libraries
target_link_libraries(Projet glfw3 ${OPENGL_LIBRARIES} glew32s glm::glm)
target_compile_definitions(Projet PRIVATE GLEW_STATIC)

# Set working directory for Visual Studio (optional)
set_target_properties(Projet PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")

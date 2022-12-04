cmake_minimum_required(VERSION 3.23)
project(CG_Final)

set(CMAKE_CXX_STANDARD 14)

include_directories(Project1)

find_package(OpenGL REQUIRED)

# GLM
find_package(glm REQUIRED)
message(STATUS "GLM included at ${GLM_INCLUDE_DIR}")

# Freeglut
find_package(FreeGLUT REQUIRED)
message(STATUS "GLUT included at ${FreeGLUT_INCLUDE_DIR}")

# Glew
find_package(GLEW REQUIRED)
message(STATUS "GLEW included at ${GLEW_INCLUDE_DIR}")

# GLFW
find_package(glfw3 REQUIRED)
message(STATUS "Found GLFW3 in ${GLFW3_INCLUDE_DIR}")

add_executable(CG_Final
        Project1/Animation.cpp
        Project1/Animation.h
        Project1/Colour.cpp
        Project1/Colour.h
        Project1/glsl.cpp
        Project1/glsl.h
        Project1/LightSource.h
        Project1/main.cpp
        Project1/Material.h
        Project1/MathsHelper.cpp
        Project1/MathsHelper.h
        Project1/ObjectFactory.cpp
        Project1/ObjectFactory.h
        Project1/objloader.cpp
        Project1/objloader.h
        Project1/SceneObject.cpp
        Project1/SceneObject.h
        Project1/Shader.h
        Project1/texture.cpp
        Project1/texture.h)

file(COPY Project1/Objects DESTINATION ${CMAKE_BINARY_DIR})
file(COPY Project1/Textures DESTINATION ${CMAKE_BINARY_DIR})
file(COPY Project1/fragmentshader_matte.frag DESTINATION ${CMAKE_BINARY_DIR})
file(COPY Project1/fragmentshader_shiny.frag DESTINATION ${CMAKE_BINARY_DIR})
file(COPY Project1/vertexshader.vert DESTINATION ${CMAKE_BINARY_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${OPENGL_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${GLM_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${GLEW_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE ${OPENGL_LIBRARIES} FreeGLUT::freeglut glm glfw GLEW)
cmake_minimum_required(VERSION 3.17)
project(jbtreug)

set(CMAKE_CXX_STANDARD 17)

#copy shaders source to build dir
configure_file(vertexS.glsl vertexS.glsl COPYONLY)
configure_file(fragmentS.glsl fragmentS.glsl COPYONLY)

add_link_options(-lX11 -lGL)
add_executable(jbtreug main.cpp shader_import.cpp geometry.cpp triangles_manager.cpp)
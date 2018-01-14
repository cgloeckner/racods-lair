cmake_minimum_required (VERSION 3.0)
project (racod)

# build SFML
add_subdirectory(sfml)

# build Thor
add_subdirectory(thor)

# build ImGui
add_library(imgui imgui/imgui.cpp imgui/imgui_draw.cpp imgui/imgui-SFML.cpp)

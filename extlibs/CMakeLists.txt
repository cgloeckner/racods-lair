cmake_minimum_required (VERSION 3.0)
project (racod)

# build SFML
add_subdirectory(sfml)

# build Thor
include_directories(thor/include)
include_directories(thor/extlibs/aurora/include)
add_subdirectory(thor)

# include ImGui from submodule (custom repo, due to imconfig.h)
include_directories(imgui)
add_library(imgui imgui/imgui.cpp imgui/imgui_draw.cpp imgui/imgui-SFML.cpp)

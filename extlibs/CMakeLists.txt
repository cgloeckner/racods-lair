cmake_minimum_required (VERSION 3.0)
project (racod)

macro(CHECK_EXTLIB file)
	if (NOT EXISTS ${file})
		message(FATAL_ERROR "This repository is incomplete, initialize submodules using:\n  git submodule update --init --recursive")
	endif()
endmacro()

CHECK_EXTLIB(${CMAKE_CURRENT_SOURCE_DIR}/sfml/CMakeLists.txt)
CHECK_EXTLIB(${CMAKE_CURRENT_SOURCE_DIR}/thor/CMakeLists.txt)
CHECK_EXTLIB(${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp) # note: no cmake there

# build SFML
add_subdirectory(sfml)

# build Thor
add_subdirectory(thor)

# build ImGui
add_library(imgui imgui/imgui.cpp imgui/imgui_draw.cpp imgui/imgui-SFML.cpp)

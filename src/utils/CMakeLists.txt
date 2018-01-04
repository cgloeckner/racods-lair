cmake_minimum_required (VERSION 3.0)
project (racod)

# build lib from this src
set(src
	animation_utils.cpp
	algorithm.cpp
	assert.cpp
	binary_utils.cpp
	camera.cpp
	event_system.cpp
	fader.cpp
	filesystem.cpp
	input_mapper.cpp
	lighting_system.cpp
	logger.cpp
	lua_utils.cpp
	menu.cpp
	ortho_tile.cpp
	pathfinder.cpp
	scope_guard.cpp
	unionfind.cpp
	verifier.cpp
)
add_library(utils ${src})


cmake_minimum_required (VERSION 2.8)
project (racod)

# build lib from this src
set(src
    algorithm.cpp
	animation.cpp
	collision.cpp
	common.cpp
	dungeon.cpp
	entity.cpp
	event.cpp
	focus.cpp
	movement.cpp
	music.cpp
	render.cpp
	sound.cpp
	teleport.cpp
)
add_library(core ${src})

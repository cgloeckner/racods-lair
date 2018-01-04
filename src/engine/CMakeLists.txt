cmake_minimum_required (VERSION 2.8)
project (racod)

# build lib from this src
set(src
	ai.cpp
	avatar.cpp
	behavior.cpp
	common.cpp
	engine.cpp
	event.cpp
	physics.cpp
	savemanager.cpp
	ui.cpp
)
add_library(engine ${src})

cmake_minimum_required (VERSION 2.8)
project (racod)

# build lib from this src
set(src
	audio.cpp
	autocam.cpp
	builder.cpp
	common.cpp
	entity.cpp
	event.cpp
	factory.cpp
	generator.cpp
	hud.cpp
	item.cpp
	lua.cpp
	mod.cpp
	navigator.cpp
	path.cpp
	player.cpp
	powerup.cpp
	resources.cpp
	script.cpp
	session.cpp
	tracer.cpp
	visuals.cpp
)
add_library(game ${src})

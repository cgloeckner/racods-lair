cmake_minimum_required (VERSION 2.8)
project (racod)

# build lib from this src
set(src
	action.cpp
	algorithm.cpp
	balance.cpp
	combat.cpp
	common.cpp
	delay.cpp
	entity.cpp
	effect.cpp
	event.cpp
	gameplay.cpp
	input.cpp
	interact.cpp
	item.cpp
	perk.cpp
	player.cpp
	projectile.cpp
	resources.cpp
	session.cpp
	spritepacker.cpp
	stats.cpp
	quickslot.cpp
)
add_library(rpg ${src})

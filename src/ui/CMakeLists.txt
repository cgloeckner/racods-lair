cmake_minimum_required (VERSION 2.8)
project (racod)

# build lib from this src
set(src
	animation.cpp
	button.cpp
	checkbox.cpp
	common.cpp
	floatingtext.cpp
	imgui.cpp
	../../extlibs/imgui/imgui.cpp
	../../extlibs/imgui/imgui_draw.cpp
	../../extlibs/imgui/imgui-SFML.cpp
	input.cpp
	notificationtext.cpp
	playerhud.cpp
	select.cpp
	statusbar.cpp
	systemgraph.cpp
)
add_library(ui ${src})


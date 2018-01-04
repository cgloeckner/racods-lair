cmake_minimum_required (VERSION 2.8)
project (racod)

# build lib from this src
set(src
	camera_editor.cpp
	character_viewer.cpp
	common.cpp
	controls_viewer.cpp
	controls_editor.cpp
	game.cpp
	app_launcher.cpp
	game_launcher.cpp
	loadthread.cpp
	lobby.cpp
	logo.cpp
	pause.cpp
	profile_creator.cpp
	resources.cpp
	savegame_menu.cpp
	settings.cpp
	story.cpp
	title.cpp
	
	tool/inspector.cpp
	tool/modverify.cpp
	tool/roomeditor.cpp
	tool/savegameviewer.cpp
	tool/spritepacker.cpp
	tool/spriteviewer.cpp
	tool/testmode.cpp
	tool/toolmenu.cpp
)
add_library(state ${src})


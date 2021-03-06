cmake_minimum_required (VERSION 2.8)
project (racod)

if (CMAKE_BUILD_TYPE AND "${CMAKE_BUILD_TYPE}" STREQUAL "RELEASE")
	set(RELEASE_BUILD 1)
else()
	set(RELEASE_BUILD 0)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set(IS_WINDOWS 1)
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(IS_64_BIT 1)
endif()

# apply some flags
if (RELEASE_BUILD)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -O2 -DTHOR_EXPORTS")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -g -O0")
endif()

set(TEST_COMPILE_FLAGS "-DUNIT_TEST")

if(IS_WINDOWS)
	# MinGW compatibility
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmax-errors=20")
	set(TEST_LINK_FLAGS " ")
else()
	# LLVM compatbility
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ferror-limit=20 -Werror -Wall")
	if (RELEASE_BUILD)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
	else()
		set(EXT_COMPILE_FLAGS "-w")
	endif()
	
	if(IS_64_BIT)
		# build with ubsan
		set(TEST_COMPILE_FLAGS "${TEST_COMPILE_FLAGS} -fsanitize=undefined,address")
		set(TEST_LINK_FLAGS "-fsanitize=undefined,address")
	else()
		# build without ubsan (bug: undefined reference to __muloid4 or similar^^)
		set(TEST_COMPILE_FLAGS "${TEST_COMPILE_FLAGS} -fsanitize=address")
		set(TEST_LINK_FLAGS "-fsanitize=address")
	endif()
endif()

# include local headers
include_directories(include)

# include third party libraries
include_directories(SYSTEM ext/include)
include_directories(SYSTEM ext/include/imgui)

# list all dependencies
if(IS_WINDOWS)
	set(SFML_LIBS
		sfml-system-2 sfml-graphics-2 sfml-window-2 sfml-audio-2 sfml-network-2 # SFML >= 2.2, "sfml-system-2.dll" etc.
	)
	if(RELEASE_BUILD)
		set(RACOD_DEPENDENCIES
			sfml-system-2 sfml-graphics-2 sfml-window-2 sfml-audio-2 sfml-network-2 # SFML >= 2.2, "sfml-system-2.dll" etc.
			Imm32 opengl32 # imgui dependencies
			lua52 # lua52.dll
		)
	else()
		set(RACOD_DEPENDENCIES
			sfml-system-2 sfml-graphics-2 sfml-window-2 sfml-audio-2 sfml-network-2 # SFML >= 2.2, "sfml-system-2.dll" etc.
			lua52 # lua52.dll
			ext_boost_lib ext_thor_lib ext_imgui_lib
		)
	endif()
else()
	set(SFML_LIBS
		sfml-system sfml-graphics sfml-window sfml-audio sfml-network # SFML >= 2.2
	)
	if(RELEASE_BUILD)
		set(RACOD_DEPENDENCIES
			sfml-system sfml-graphics sfml-window sfml-audio sfml-network # SFML >= 2.2
			GL # imgui dependencies
			lua5.2 # liblua5.2.so
			X11
		)
	else()
		set(RACOD_DEPENDENCIES
			sfml-system sfml-graphics sfml-window sfml-audio sfml-network # SFML >= 2.2
			lua5.2 # liblua5.2.so
			X11
			ext_boost_lib ext_thor_lib ext_imgui_lib
		)
	endif()
endif()

# -----------------------------------------------------------------------------
# Group third-party source

set(EXT_BOOST_SOURCE
	ext/src/boost/system/error_code.cpp
	ext/src/boost/filesystem/codecvt_error_category.cpp
	ext/src/boost/filesystem/operations.cpp
	ext/src/boost/filesystem/path.cpp
	ext/src/boost/filesystem/path_traits.cpp
	ext/src/boost/filesystem/portability.cpp
	ext/src/boost/filesystem/unique_path.cpp
	ext/src/boost/filesystem/utf8_codecvt_facet.cpp
	ext/src/boost/filesystem/windows_file_codecvt.cpp
)

set(EXT_THOR_SOURCE
	ext/src/Thor/Action.cpp
	ext/src/Thor/ActionOperations.cpp
	ext/src/Thor/Affectors.cpp
	ext/src/Thor/Arrow.cpp
	ext/src/Thor/BigSprite.cpp
	ext/src/Thor/BigTexture.cpp
	ext/src/Thor/CallbackTimer.cpp
	ext/src/Thor/ColorAnimation.cpp
	ext/src/Thor/ColorGradient.cpp
	ext/src/Thor/ConcaveShape.cpp
	ext/src/Thor/Connection.cpp
	ext/src/Thor/Distributions.cpp
	ext/src/Thor/Emitters.cpp
	ext/src/Thor/FadeAnimation.cpp
	ext/src/Thor/FrameAnimation.cpp
	ext/src/Thor/InputNames.cpp
	ext/src/Thor/Joystick.cpp
	ext/src/Thor/Particle.cpp
	ext/src/Thor/ParticleSystem.cpp
	ext/src/Thor/Random.cpp
	ext/src/Thor/Shapes.cpp
	ext/src/Thor/StopWatch.cpp
	ext/src/Thor/Timer.cpp
	ext/src/Thor/ToString.cpp
	ext/src/Thor/Triangulation.cpp
	ext/src/Thor/Trigonometry.cpp
	ext/src/Thor/UniformAccess.cpp
)

set(EXT_IMGUI_SOURCE
	ext/src/imgui/imgui.cpp
	ext/src/imgui/imgui_draw.cpp
	ext/src/imgui/imgui-SFML.cpp
)

# ----------------------------------------------------------------------------
# Group game source

set(RACOD_UTILS_SOURCE
	src/utils/animation_utils.cpp
	src/utils/algorithm.cpp
	src/utils/assert.cpp
	src/utils/binary_utils.cpp
	src/utils/camera.cpp
	src/utils/event_system.cpp
	src/utils/fader.cpp
	src/utils/filesystem.cpp
	src/utils/input_mapper.cpp
	src/utils/lighting_system.cpp
	src/utils/logger.cpp
	src/utils/lua_utils.cpp
	src/utils/menu.cpp
	src/utils/ortho_tile.cpp
	src/utils/pathfinder.cpp
	src/utils/scope_guard.cpp
	src/utils/unionfind.cpp
	src/utils/verifier.cpp
)
set(RACOD_CORE_SOURCE
	src/core/algorithm.cpp
	src/core/animation.cpp
	src/core/collision.cpp
	src/core/common.cpp
	src/core/dungeon.cpp
	src/core/entity.cpp
	src/core/event.cpp
	src/core/focus.cpp
	src/core/movement.cpp
	src/core/music.cpp
	src/core/render.cpp
	src/core/sound.cpp
	src/core/teleport.cpp
)
set(RACOD_RPG_SOURCE
	src/rpg/action.cpp
	src/rpg/algorithm.cpp
	src/rpg/balance.cpp
	src/rpg/combat.cpp
	src/rpg/common.cpp
	src/rpg/delay.cpp
	src/rpg/entity.cpp
	src/rpg/effect.cpp
	src/rpg/event.cpp
	src/rpg/gameplay.cpp
	src/rpg/input.cpp
	src/rpg/interact.cpp
	src/rpg/item.cpp
	src/rpg/perk.cpp
	src/rpg/player.cpp
	src/rpg/projectile.cpp
	src/rpg/resources.cpp
	src/rpg/session.cpp
	src/rpg/spritepacker.cpp
	src/rpg/stats.cpp
	src/rpg/quickslot.cpp
)
set(RACOD_UI_SOURCE
	src/ui/animation.cpp
	src/ui/button.cpp
	src/ui/checkbox.cpp
	src/ui/common.cpp
	src/ui/floatingtext.cpp
	src/ui/imgui.cpp
	src/ui/input.cpp
	src/ui/notificationtext.cpp
	src/ui/playerhud.cpp
	src/ui/select.cpp
	src/ui/statusbar.cpp
	src/ui/systemgraph.cpp
	src/sfml-fix.cpp
)
set(RACOD_GAME_SOURCE
	src/game/audio.cpp
	src/game/autocam.cpp
	src/game/builder.cpp
	src/game/common.cpp
	src/game/entity.cpp
	src/game/event.cpp
	src/game/factory.cpp
	src/game/generator.cpp
	src/game/hud.cpp
	src/game/item.cpp
	src/game/lua.cpp
	src/game/mod.cpp
	src/game/navigator.cpp
	src/game/path.cpp
	src/game/player.cpp
	src/game/powerup.cpp
	src/game/resources.cpp
	src/game/script.cpp
	src/game/session.cpp
	src/game/tracer.cpp
	src/game/visuals.cpp
)
set(RACOD_ENGINE_SOURCE
	src/engine/ai.cpp
	src/engine/avatar.cpp
	src/engine/behavior.cpp
	src/engine/common.cpp
	src/engine/engine.cpp
	src/engine/event.cpp
	src/engine/physics.cpp
	src/engine/savemanager.cpp
	src/engine/ui.cpp
)
set(RACOD_STATE_SOURCE
	src/state/camera_editor.cpp
	src/state/character_viewer.cpp
	src/state/common.cpp
	src/state/controls_viewer.cpp
	src/state/controls_editor.cpp
	src/state/game.cpp
	src/state/app_launcher.cpp
	src/state/game_launcher.cpp
	src/state/loadthread.cpp
	src/state/lobby.cpp
	src/state/logo.cpp
	src/state/pause.cpp
	src/state/profile_creator.cpp
	src/state/resources.cpp
	src/state/savegame_menu.cpp
	src/state/settings.cpp
	src/state/story.cpp
	src/state/title.cpp
	
	src/state/tool/inspector.cpp
	src/state/tool/modverify.cpp
	src/state/tool/roomeditor.cpp
	src/state/tool/savegameviewer.cpp
	src/state/tool/spritepacker.cpp
	src/state/tool/spriteviewer.cpp
	src/state/tool/testmode.cpp
	src/state/tool/toolmenu.cpp
)
set(RACOD_MAIN_SOURCE
	src/main.cpp
)

# ----------------------------------------------------------------------------
# Group test suite source

set(RACOD_TEST_MAIN_SOURCE
	test_suite/main.cpp
	test_suite/sfml_system.cpp
)

set(RACOD_UTILS_TEST_SOURCE
	test_suite/utils/atlas.hpp
	test_suite/utils/algorithm.cpp
	test_suite/utils/animation_utils.cpp
	test_suite/utils/assert.cpp
	test_suite/utils/binary_utils.cpp
	test_suite/utils/camera.cpp
	test_suite/utils/component_system.cpp
	test_suite/utils/delay_system.cpp
	test_suite/utils/enum_map.cpp
	test_suite/utils/enum_utils.cpp
	test_suite/utils/event_system.cpp
	test_suite/utils/lighting_system.cpp
	test_suite/utils/logger.cpp
	test_suite/utils/lua_utils.cpp
	test_suite/utils/ortho_tile.cpp
	test_suite/utils/input_mapper.cpp
	test_suite/utils/keybinding.cpp
	test_suite/utils/pathfinder.cpp
	test_suite/utils/priority_queue.cpp
	test_suite/utils/resource_cache.cpp
	test_suite/utils/scope_guard.cpp
	test_suite/utils/spatial_scene.cpp
	test_suite/utils/tiling.hpp
	test_suite/utils/unionfind.cpp
	test_suite/utils/xml_utils.cpp
	test_suite/utils/verifier.cpp
)
set(RACOD_CORE_TEST_SOURCE
	test_suite/core/animation.cpp
	test_suite/core/algorithm.cpp
	test_suite/core/collision.cpp
	test_suite/core/focus.cpp
	test_suite/core/movement.cpp
	test_suite/core/physics_integration.cpp
	test_suite/core/render.cpp
	test_suite/core/sound.cpp
	test_suite/core/teleport.cpp
)
set(RACOD_RPG_TEST_SOURCE
	test_suite/rpg/action.cpp
	test_suite/rpg/algorithm.cpp
	test_suite/rpg/balance.cpp
	test_suite/rpg/combat.cpp
	test_suite/rpg/delay.cpp
	test_suite/rpg/effect.cpp
	test_suite/rpg/gameplay_integration.cpp
	test_suite/rpg/input.cpp
	test_suite/rpg/interact.cpp
	test_suite/rpg/item.cpp
	test_suite/rpg/perk.cpp
	test_suite/rpg/player.cpp
	test_suite/rpg/projectile.cpp
	test_suite/rpg/resources.cpp
	test_suite/rpg/stats.cpp
	test_suite/rpg/quickslot.cpp
)
set(RACOD_GAME_TEST_SOURCE
	test_suite/game/audio.cpp
	test_suite/game/autocam.cpp
	test_suite/game/builder.cpp
	test_suite/game/common.cpp
	test_suite/game/generator.cpp
	test_suite/game/factory.cpp
	test_suite/game/lua.cpp
	test_suite/game/mod.cpp
	test_suite/game/navigator.cpp
	test_suite/game/path.cpp
	test_suite/game/powerup.cpp
	test_suite/game/resources.cpp
	test_suite/game/script.cpp
	test_suite/game/tracer.cpp
	test_suite/game/visuals.cpp
)
set(RACOD_ENGINE_TEST_SOURCE
)
set(RACOD_STATE_TEST_SOURCE
	test_suite/state/common.cpp
	test_suite/state/resources.cpp
)

if (RELEASE_BUILD)
	set(RACOD_FULL_SOURCE
		"${EXT_BOOST_SOURCE}"
		"${EXT_THOR_SOURCE}"
		"${EXT_IMGUI_SOURCE}"
		"${RACOD_UTILS_SOURCE}"
		"${RACOD_CORE_SOURCE}"
		"${RACOD_RPG_SOURCE}"
		"${RACOD_UI_SOURCE}"
		"${RACOD_ENGINE_SOURCE}"
		"${RACOD_GAME_SOURCE}"
		"${RACOD_STATE_SOURCE}"
		"${RACOD_MAIN_SOURCE}"
	)
else()
	set(RACOD_FULL_SOURCE
		"${RACOD_UTILS_SOURCE}"
		"${RACOD_CORE_SOURCE}"
		"${RACOD_RPG_SOURCE}"
		"${RACOD_UI_SOURCE}"
		"${RACOD_ENGINE_SOURCE}"
		"${RACOD_GAME_SOURCE}"
		"${RACOD_STATE_SOURCE}"
		"${RACOD_MAIN_SOURCE}"
	)
endif()

# ----------------------------------------------------------------------------
# Bundle code to shared libs for module tests

macro(prebuild_module_test target sources cflags)
	add_library(${target} SHARED ${sources})
	set_target_properties(${target} PROPERTIES COMPILE_FLAGS "${cflags}")
	set_target_properties(${target} PROPERTIES LINK_FLAGS ${TEST_LINK_FLAGS})
endmacro(prebuild_module_test)

if (NOT RELEASE_BUILD)
	prebuild_module_test(ext_boost_lib "${EXT_BOOST_SOURCE}" "${EXT_COMPILE_FLAGS}")
	prebuild_module_test(ext_thor_lib "${EXT_THOR_SOURCE}" "${EXT_COMPILE_FLAGS} -DTHOR_EXPORTS")
	prebuild_module_test(ext_imgui_lib "${EXT_IMGUI_SOURCE}" "${EXT_COMPILE_FLAGS}")
	prebuild_module_test(racod_test_lib "${RACOD_TEST_MAIN_SOURCE}" "${EXT_COMPILE_FLAGS}")
	prebuild_module_test(racod_utils_lib "${RACOD_UTILS_SOURCE}" "${TEST_COMPILE_FLAGS}")
	prebuild_module_test(racod_core_lib "${RACOD_CORE_SOURCE}" "${TEST_COMPILE_FLAGS}")
	prebuild_module_test(racod_rpg_lib "${RACOD_RPG_SOURCE}" "${TEST_COMPILE_FLAGS}")
	prebuild_module_test(racod_ui_lib "${RACOD_UI_SOURCE}" "${TEST_COMPILE_FLAGS}")
	prebuild_module_test(racod_game_lib "${RACOD_GAME_SOURCE}" "${TEST_COMPILE_FLAGS}")
	prebuild_module_test(racod_engine_lib "${RACOD_ENGINE_SOURCE}" "${TEST_COMPILE_FLAGS}")
	prebuild_module_test(racod_state_lib "${RACOD_STATE_SOURCE}" "${TEST_COMPILE_FLAGS}")
endif()

if(IS_WINDOWS)
	# link tmp dlls against their deps
	if (NOT RELEASE_BUILD)
		target_link_libraries(ext_thor_lib ${SFML_LIBS})
		target_link_libraries(ext_imgui_lib Imm32 opengl32 ${SFML_LIBS})
		target_link_libraries(racod_test_lib ${SFML_LIBS})
		target_link_libraries(racod_utils_lib racod_test_lib ext_boost_lib ext_thor_lib ${SFML_LIBS} lua52)
		target_link_libraries(racod_core_lib racod_utils_lib)
		target_link_libraries(racod_rpg_lib racod_core_lib)
		target_link_libraries(racod_ui_lib racod_utils_lib ext_imgui_lib)
		target_link_libraries(racod_game_lib racod_rpg_lib racod_ui_lib)
		target_link_libraries(racod_engine_lib racod_game_lib ${SFML_LIBS})
		target_link_libraries(racod_state_lib racod_game_lib racod_engine_lib ext_imgui_lib)
	endif()
endif()

set(RACOD_UTILS_TEST_DEPS
	racod_test_lib
	racod_utils_lib
)
set(RACOD_CORE_TEST_DEPS
	racod_test_lib
	racod_utils_lib
	racod_core_lib
)
set(RACOD_RPG_TEST_DEPS
	racod_test_lib
	racod_utils_lib
	racod_core_lib
	racod_rpg_lib
)
set(RACOD_GAME_TEST_DEPS
	racod_test_lib
	racod_utils_lib
	racod_core_lib
	racod_rpg_lib
	racod_ui_lib
	racod_game_lib
)
set(RACOD_STATE_TEST_DEPS
	racod_test_lib
	racod_utils_lib
	racod_core_lib
	racod_rpg_lib
	racod_ui_lib
	racod_game_lib
	racod_engine_lib
	racod_state_lib
)

# ----------------------------------------------------------------------------
# Build and run module tests

macro(build_module_test target sources deps)
	add_executable(${target} ${sources})
	set_target_properties(${target} PROPERTIES COMPILE_FLAGS ${TEST_COMPILE_FLAGS} )
	set_target_properties(${target} PROPERTIES LINK_FLAGS ${TEST_LINK_FLAGS})
	target_link_libraries(${target}
		${RACOD_DEPENDENCIES}
		${deps}
	)
	if(IS_WINDOWS)
		add_custom_command(TARGET ${target}
			POST_BUILD
			COMMAND ${target}.exe --output-on-failure
		)
	else()
		add_custom_command(TARGET ${target}
			POST_BUILD
			COMMAND ASAN_SYMBOLIZER_PATH=/usr/sbin/llvm-symbolizer ./${target} --output-on-failure
		)
	endif()
endmacro(build_module_test)

if (NOT RELEASE_BUILD)
	build_module_test(racod_utils_test "${RACOD_UTILS_TEST_SOURCE}" "${RACOD_UTILS_TEST_DEPS}")
	build_module_test(racod_core_test "${RACOD_CORE_TEST_SOURCE}" "${RACOD_CORE_TEST_DEPS}")
	build_module_test(racod_rpg_test "${RACOD_RPG_TEST_SOURCE}" "${RACOD_RPG_TEST_DEPS}")
	#	build_module_test(racod_ui_test "${RACOD_UI_TEST_SOURCE}" "${RACOD_UI_TEST_DEPS}")
	build_module_test(racod_game_test "${RACOD_GAME_TEST_SOURCE}" "${RACOD_GAME_TEST_DEPS}")
	#	build_module_test(racod_engine_test "${RACOD_ENGINE_TEST_SOURCE}" "${RACOD_ENGINE_TEST_DEPS}")
	build_module_test(racod_state_test "${RACOD_STATE_TEST_SOURCE}" "${RACOD_STATE_TEST_DEPS}")
endif()

# -----------------------------------------------------------------------------
# Build debugging game executable

macro(build_game)
	add_executable(racod_game ${RACOD_FULL_SOURCE})
	#set_target_properties(racod_game PROPERTIES COMPILE_FLAGS " ")
	set_target_properties(racod_game PROPERTIES LINK_FLAGS "-pthread")
	target_link_libraries(racod_game ${RACOD_DEPENDENCIES})
endmacro(build_game)

build_game()

# ----------------------------------------------------------------------------
# Setup dependencies

if (RELEASE_BUILD)
	#add_dependencies(racod_game ext_boost_lib ext_thor_lib)
else()
	add_dependencies(racod_core_lib racod_utils_test)
	add_dependencies(racod_rpg_lib racod_core_test)
	add_dependencies(racod_game_lib racod_rpg_test)
	add_dependencies(racod_engine_lib racod_game_test)
	add_dependencies(racod_state_lib racod_game_test)
	# tmp:
	add_dependencies(racod_game racod_state_test)
endif()

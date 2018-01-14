cmake_minimum_required (VERSION 3.0)
project (racod)

# build test library
add_library(test_lib main.cpp sfml_system.cpp)

# macro for building and running Unit Tests per module
macro(test_module module deps)
	# query all *.cpp files
	file(GLOB_RECURSE src "${module}/*.cpp")
	
	# build exectuable
	add_executable(${module}_test ${src})
	
	# link external libs, test program and module libs
	target_link_libraries(${module}_test ${FULL_DEPS} test_lib ${deps})
	
	# run it
	# todo: windows support?
	add_custom_command(TARGET ${module}_test POST_BUILD
		COMMAND ./${module}_test
	)
endmacro(test_module)

set(utils_test_deps	utils)
set(core_test_deps	utils	core)
set(rpg_test_deps	utils	core	rpg)
set(game_test_deps	utils	ui		core	rpg	game)
set(state_test_deps	utils	ui		core	rpg	game	engine	state)

# build all unit tests (linking against some module libs)
test_module(utils	"${utils_test_deps}")
#test_module(ui) # not implemented yet
test_module(core	"${core_test_deps}")
test_module(rpg		"${rpg_test_deps}")
test_module(game	""${game_test_deps}")
#test_module(engine) # not implemented yet
test_module(state	"${state_test_deps}")

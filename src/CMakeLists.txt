cmake_minimum_required (VERSION 3.0)
project (racod)

# build main 
add_executable(racod_game main.cpp)

# link external libs
target_link_libraries(racod_game ${FULL_DEPS})


# macro for building a module
macro(build_module module)
    # query all *.cpp files
    file(GLOB src "${module}/*.cpp")
    
    # build lib
    add_library(${module} ${src})
    
    # link this lib to the game
    target_link_libraries(racod_game ${module})
    
endmacro(build_module)


# build all modules
build_module(utils)
build_module(core)
build_module(rpg)
build_module(ui)
build_module(game)
build_module(engine)
build_module(state)

add_dependencies(ui utils)
add_dependencies(core utils)
add_dependencies(rpg core)
add_dependencies(game rpg)
add_dependencies(engine game)
add_dependencies(state engine)

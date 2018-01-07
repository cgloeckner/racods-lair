cmake_minimum_required (VERSION 2.8)
project (racod)

# add subdirs (they build libs)
add_subdirectory(utils)
add_subdirectory(core)
add_subdirectory(rpg)
add_subdirectory(ui)
add_subdirectory(game)
add_subdirectory(engine)
add_subdirectory(state)

# build this src
set(src
    main.cpp
)
add_executable(racod_game ${src})

# link subdirs' libs
target_link_libraries(racod_game core)
target_link_libraries(racod_game engine)
target_link_libraries(racod_game game)
target_link_libraries(racod_game rpg)
target_link_libraries(racod_game state)
target_link_libraries(racod_game ui)
target_link_libraries(racod_game utils)

# link depending libs
target_link_libraries(racod_game sfml-system sfml-graphics sfml-window sfml-audio sfml-network)
target_link_libraries(racod_game thor)
target_link_libraries(racod_game ${LUA_LIBRARY})
target_link_libraries(racod_game boost_filesystem)


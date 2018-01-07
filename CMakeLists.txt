cmake_minimum_required (VERSION 3.0)
project (racod)

set(CMAKE_CXX_STANDARD 14)

# include and build SFML from submodule
include_directories(extlibs/sfml/include)
add_subdirectory(extlibs/sfml)

# include and build Thor from submodule
include_directories(extlibs/thor/include)
include_directories(extlibs/thor/extlibs/aurora/include)
add_subdirectory(extlibs/thor)

# include Sol2 from submodule (using the specified lua version)
include_directories(${LUA_INCLUDE_DIR})
include_directories(extlibs/sol2)

# include ImGui from submodule (custom repo, due to imconfig.h)
include_directories(extlibs/imgui)

# build Racod's Lair
include_directories(include)
add_subdirectory(src)


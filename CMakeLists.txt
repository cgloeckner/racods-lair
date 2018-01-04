cmake_minimum_required (VERSION 3.0)
project (racod)

set(CMAKE_CXX_STANDARD 14)

# build depending libs
include_directories(extlibs/sfml/include)
add_subdirectory(extlibs/sfml)

include_directories(extlibs/thor/include)
include_directories(extlibs/thor/extlibs/aurora/include)
add_subdirectory(extlibs/thor)

include_directories(${LUA_INCLUDE_DIR})
include_directories(extlibs/sol2)
include_directories(extlibs/sol2/sol)
add_subdirectory(extlibs/sol2) # does this build anything at all?

include_directories(extlibs) # imgui isn't using a dedicatad imgui/ folder -- see #include <imgui/foo.hpp>

# build Racod's Lair
include_directories(include)
add_subdirectory(src)


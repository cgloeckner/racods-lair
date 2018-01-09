cmake_minimum_required (VERSION 3.0)
project (racod)

set(CMAKE_CXX_STANDARD 14)
set(FULL_DEPS sfml-system sfml-graphics sfml-window sfml-audio sfml-network thor
    boost_filesystem ${LUA_LIBRARY})

if (RELEASE_BUILD)
else()
    add_definitions(-DUNIT_TEST)
endif()

include(ProcessorCount)
ProcessorCount(N)
if(NOT N EQUAL 0)
  set(CTEST_BUILD_FLAGS -j${N})
  set(ctest_test_args ${ctest_test_args} PARALLEL_LEVEL ${N})
endif()

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

include_directories(include)

if (RELEASE_BUILD)
else()
    # build Unit Tests
    add_subdirectory(test_suite)
endif()

# build Racod's Lair
add_subdirectory(src)


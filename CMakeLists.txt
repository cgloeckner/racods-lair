cmake_minimum_required (VERSION 3.0)
project (racod)

set(CMAKE_CXX_STANDARD 14)
set(FULL_DEPS sfml-system sfml-graphics sfml-window sfml-audio sfml-network
	thor imgui
	boost_filesystem ${LUA_LIBRARY}
)

# required for proper sol2
include_directories(${LUA_INCLUDE_DIR})

# ----------------------------------------------------------------------------------------
# extlibs --------------------------------------------------------------------------------

include_directories(extlibs/sfml/include)
include_directories(extlibs/thor/include)
include_directories(extlibs/thor/extlibs/aurora/include)
include_directories(extlibs/sol2)
include_directories(extlibs/imgui) # custom repo due to imconfig.h

# build extlibs
add_subdirectory(extlibs)

# ----------------------------------------------------------------------------------------
# Racod's Lair ---------------------------------------------------------------------------

include_directories(include)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
	message(STATUS "Racod's Lair -- Release Build -----------------------------------------")
	# optimize build
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
	
elseif(CMAKE_BUILD_TYPE STREQUAL "Test")
	message(STATUS "Racod's Lair -- Unit Test Build ---------------------------------------")
	set(UNIT_TEST 1)
	# enable unit testing
	add_definitions(-DUNIT_TEST)
	
	# enable debug options
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
	
	# enable sanitizers
	set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS}  -fsanitize=undefined,address,leak -fno-omit-frame-pointer")
	set(CMAKE_LINK_FLAGS "${CMAKE_LINK_FLAGS} -fsanitize=undefined,address,leak ASAN_OPTIONS=detect_leaks=1")
	
	# build test suites
	add_subdirectory(test_suite)
	
else()
	message(STATUS "Racod's Lair -- Debug Build -------------------------------------------")
	# enable debug options
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
	
endif()
message(STATUS "-----------------------------------------------------------------------")

# build Racod's Lair
add_subdirectory(src)
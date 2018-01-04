cmake_minimum_required (VERSION 2.8)
project (racod)

set(SRC
	system/error_code.cpp
	filesystem/codecvt_error_category.cpp
	filesystem/operations.cpp
	filesystem/path.cpp
	filesystem/path_traits.cpp
	filesystem/portability.cpp
	filesystem/unique_path.cpp
	filesystem/utf8_codecvt_facet.cpp
	filesystem/windows_file_codecvt.cpp
)
add_library(boost ${src})

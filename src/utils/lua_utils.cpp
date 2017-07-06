#include <iostream>
#include <utils/lua_utils.hpp>

namespace utils {

Script::Script()
	: lua{}
	, loaded{false}
	, filename{} {
	lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::table);
}

bool Script::loadFromMemory(std::string const& string) {
	try {
		lua.script(string);
	} catch (sol::error const & err) {
		std::cerr << "Failed to load lua script from memory: "
				  << err.what() << "\n";
		return false;
	}
	loaded = true;
	filename = "";
	return true;
}

bool Script::loadFromFile(std::string const& fname) {
	try {
		lua.script_file(fname);
	} catch (sol::error const & err) {
		std::cerr << "Failed to load lua script from file: "
				  << err.what() << "\n";
		return false;
	}
	loaded = true;
	filename = fname;
	return true;
}

std::string Script::getFilename() const {
	return filename;
}

}  // ::utils

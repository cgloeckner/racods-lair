#include <iostream>

#include <utils/enum_utils.hpp>

namespace utils {

template <typename T>
void Script::bind() {
	ASSERT(!loaded);
	Binder<T>::execute(lua);
}

template <typename... Args>
void Script::operator()(std::string const& name, Args&&... args) const {
	try {
		get<sol::function>(name)(std::forward<Args>(args)...);
	} catch (sol::error const& e) {
		std::cerr << "Script error: " << e.what() << "\n";
	}
}

template <typename T>
T Script::get(std::string const& ident) const {
	return lua.get<T>(ident);
}

template <typename T>
void Script::set(std::string const& ident, T const& value) {
	lua.set(ident, value);
}

}  // ::utils

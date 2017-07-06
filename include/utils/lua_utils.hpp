#pragma once
#include <utils/assert.hpp>

#include <type_traits>
#include <vector>
#include <sol2/sol.hpp>

#include <utils/enum_map.hpp>

namespace utils {

/// Specialize to bind types to the given lua state
template <typename T>
struct Binder {
	static void execute(sol::state& lua);
};

// ---------------------------------------------------------------------------

/// Instance of a script
class Script {
  private:
	sol::state lua;
	bool loaded;
	std::string filename;

  public:
	Script();

	template <typename T>
	void bind();
	
	bool loadFromMemory(std::string const& string);
	bool loadFromFile(std::string const& fname);

	std::string getFilename() const;

	template <typename... Args>
	void operator()(std::string const& name, Args&&... args) const;

	template <typename T=sol::table>
	T get(std::string const& ident) const;

	template <typename T>
	void set(std::string const& ident, T const& value);
};

}  // ::utils

// include implementation details
#include <utils/lua_utils.inl>

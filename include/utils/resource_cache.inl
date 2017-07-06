#include <utils/algorithm.hpp>

namespace utils {

template <typename T>
bool SingleResourceCache<T>::has(std::string const & filename) const {
	try {
		holder[filename];
		return true;
	} catch (thor::ResourceAccessException const & err) {
		return false;
	}
}

template <typename T>
T& SingleResourceCache<T>::get(std::string const& filename, bool reload) {
	auto strategy = thor::Resources::KnownIdStrategy::Reuse;
	if (reload) {
		strategy = thor::Resources::KnownIdStrategy::Reload;
	}
	return holder.acquire(
		filename, thor::Resources::fromFile<T>(filename), strategy);
}

template <typename... Resources>
template <typename T>
bool MultiResourceCache<Resources...>::has(std::string const& filename) const {
	static_assert(pack_contains<T, Resources...>::value, "Resource must be supported");
	return SingleResourceCache<T>::has(filename);
}

template <typename... Resources>
template <typename T>
T& MultiResourceCache<Resources...>::get(
	std::string const& filename, bool reload) {
	static_assert(
		pack_contains<T, Resources...>::value, "Resource must be supported");
	return SingleResourceCache<T>::get(filename, reload);
}

}  // ::utils

#include <utils/color.hpp>

namespace utils {

sf::Color ptrToColor(void* ptr) {
	return sf::Color{static_cast<sf::Uint32>(reinterpret_cast<unsigned long long>(ptr))};
}

} // ::utils
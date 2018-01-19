#include <Thor/Vectors.hpp>

namespace utils {

template <typename T>
T distance(sf::Vector2<T> const& u, sf::Vector2<T> const& v) {
	return thor::squaredLength(u-v);
}

}  // ::utils

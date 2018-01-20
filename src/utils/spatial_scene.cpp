#include <utils/spatial_scene.hpp>

namespace utils {

sf::IntRect toIntRect(sf::FloatRect const & range) {
	sf::IntRect rect;
	rect.left   = static_cast<int>(std::floor(range.left));
	rect.top    = static_cast<int>(std::floor(range.top));
	rect.width  = static_cast<int>(std::ceil (range.width));
	rect.height = static_cast<int>(std::ceil (range.height));
	return rect;
}

sf::IntRect toIntRect(sf::Vector2f const & center, float radius) {
	sf::IntRect rect;
	rect.left   = static_cast<int>(std::floor(center.x - radius));
	rect.top    = static_cast<int>(std::floor(center.y - radius));
	rect.width  = static_cast<int>(std::ceil (2.f * radius));
	rect.height = static_cast<int>(std::ceil (2.f * radius));
	return rect;
}

} // ::utils

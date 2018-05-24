#include <utils/assert.hpp>
#include <core/algorithm.hpp>
#include <core/collision.hpp>

namespace core {

sf::Vector2f rotate(sf::Vector2f const& vector, bool clockwise) {
	ASSERT(vector != sf::Vector2f{});
	float angle = 45.f;
	if (!clockwise) {
		angle *= -1.f;
	}
	return utils::normalize(thor::rotatedVector(vector, angle));
}

}  // ::core

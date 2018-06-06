#include <utils/assert.hpp>
#include <core/algorithm.hpp>
#include <core/collision.hpp>

namespace core {

sf::Vector2f rotate(sf::Vector2f const& vector, float angle) {
	if (vector == sf::Vector2f{}) {
		return vector;
	}
	return utils::normalize(thor::rotatedVector(vector, angle));
}

}  // ::core

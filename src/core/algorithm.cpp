#include <utils/assert.hpp>
#include <core/algorithm.hpp>
#include <core/collision.hpp>

namespace core {

sf::Vector2i rotate(sf::Vector2i const& vector, bool clockwise) {
	if (clockwise) {
		if (vector == sf::Vector2i{0, -1}) {
			return {1, -1};
		}
		if (vector == sf::Vector2i{1, -1}) {
			return {1, 0};
		}
		if (vector == sf::Vector2i{1, 0}) {
			return {1, 1};
		}
		if (vector == sf::Vector2i{1, 1}) {
			return {0, 1};
		}
		if (vector == sf::Vector2i{0, 1}) {
			return {-1, 1};
		}
		if (vector == sf::Vector2i{-1, 1}) {
			return {-1, 0};
		}
		if (vector == sf::Vector2i{-1, 0}) {
			return {-1, -1};
		}
		if (vector == sf::Vector2i{-1, -1}) {
			return {0, -1};
		}
	} else {
		if (vector == sf::Vector2i{1, -1}) {
			return {0, -1};
		}
		if (vector == sf::Vector2i{1, 0}) {
			return {1, -1};
		}
		if (vector == sf::Vector2i{1, 1}) {
			return {1, 0};
		}
		if (vector == sf::Vector2i{0, 1}) {
			return {1, 1};
		}
		if (vector == sf::Vector2i{-1, 1}) {
			return {0, 1};
		}
		if (vector == sf::Vector2i{-1, 0}) {
			return {-1, 1};
		}
		if (vector == sf::Vector2i{-1, -1}) {
			return {-1, 0};
		}
		if (vector == sf::Vector2i{0, -1}) {
			return {-1, -1};
		}
	}
	std::cout << "[Core/Algorithm] " << vector.x << "," << vector.y << "\n";
	ASSERT(false);  // should never be reached
}

void fixDirection(sf::Vector2i& vector) {
	if (vector.x > 1) {
		vector.x = 1;
	}
	if (vector.y > 1) {
		vector.y = 1;
	}
	if (vector.x < -1) {
		vector.x = -1;
	}
	if (vector.y < -1) {
		vector.y = -1;
	}
}

}  // ::core

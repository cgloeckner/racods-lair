#pragma once
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

namespace core {

template <typename Func>
void updateChunked(
	Func func, sf::Time const& elapsed, sf::Time const& steptime);

sf::Vector2i rotate(sf::Vector2i const& vector, bool clockwise);

void fixDirection(sf::Vector2i& vector);

}  // ::core

// include implementation details
#include <core/algorithm.inl>

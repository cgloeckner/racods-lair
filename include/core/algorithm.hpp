#pragma once
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

namespace core {

template <typename Func>
void updateChunked(
	Func func, sf::Time const& elapsed, sf::Time const& steptime);

sf::Vector2f rotate(sf::Vector2f const& vector, float angle=45.f);

}  // ::core

// include implementation details
#include <core/algorithm.inl>

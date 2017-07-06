#include <iostream>

#include <utils/pathfinder.hpp>

namespace utils {

PathNode::PathNode(sf::Vector2u pos)
	: pos{pos}
	, previous{nullptr}
	, f{0.f}
	, g{0.f} {
}

bool OpenlistCondition::operator()(PathNode const& lhs, PathNode const& rhs) const {
	return lhs.f > rhs.f;
}

std::size_t ClosedlistHasher::operator()(PathNode const& n) const {
	auto x = std::hash<unsigned int>()(n.pos.x);
	auto y = std::hash<unsigned int>()(n.pos.y);
	return x ^ (y << 1);
}

bool ClosedlistEqual::operator()(PathNode const& lhs, PathNode const& rhs) const {
	return lhs.pos == rhs.pos;
}

/*
// ---------------------------------------------------------------------------

PathDict::PathDict(sf::Vector2u const& grid_size)
	: grid_size{grid_size}, nodes{} {
	nodes.resize(grid_size.x * grid_size.y);
	clear();
}

void PathDict::clear() {
	sf::Vector2u pos;
	for (pos.y = 0u; pos.y < grid_size.y; ++pos.y) {
		for (pos.x = 0u; pos.x < grid_size.x; ++pos.x) {
			at(pos) = PathNode{pos};
		}
	}
}

PathNode& PathDict::at(sf::Vector2u const& pos) {
	std::size_t index = pos.x + pos.y * grid_size.x;
	return nodes.at(index);
}

// ---------------------------------------------------------------------------

CoordHash::CoordHash(sf::Vector2u const& grid_size) : grid_size{grid_size} {}

std::size_t CoordHash::range() const { return grid_size.x * grid_size.y; }

std::size_t CoordHash::operator()(sf::Vector2u const& pos) const {
	return pos.x + pos.y * grid_size.x;
}
*/

}  // ::utils

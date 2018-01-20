#include <utils/algorithm.hpp>
#include <utils/math2d.hpp>

namespace utils {

template <typename Cell, typename Entity>
SpatialCell<Cell, Entity>::SpatialCell()
	: Cell{}, entities{} {}

// ---------------------------------------------------------------------------

template <typename Entity>
AABBEntityQuery<Entity>::AABBEntityQuery(sf::Vector2f const & center, sf::Vector2f const & size)
	: range{center - size/2.f, size}
	, entities{} {
}

template <typename Entity>
sf::IntRect AABBEntityQuery<Entity>::getRange() const {
	return toIntRect(range);
}

template <typename Entity>
void AABBEntityQuery<Entity>::operator()(sf::Vector2f const & pos, std::vector<Entity> const & cell) {
	utils::append(entities, cell);
}

// ---------------------------------------------------------------------------

template <typename Entity>
CircEntityQuery<Entity>::CircEntityQuery(sf::Vector2f const & center, float radius)
	: center{center}
	, collider{}
	, entities{} {
	collider.radius = radius;
}

template <typename Entity>
sf::IntRect CircEntityQuery<Entity>::getRange() const {
	return toIntRect(center, collider.radius);
}

template <typename Entity>
void CircEntityQuery<Entity>::operator()(sf::Vector2f const & pos, std::vector<Entity> const & cell) {
	if (testPointCirc(pos, center, collider)) {
		utils::append(entities, cell);
	}
}

// ---------------------------------------------------------------------------

template <typename Cell, typename Entity, utils::GridMode Mode>
SpatialScene<Cell, Entity, Mode>::SpatialScene(SceneID id,
	sf::Texture const& tileset, sf::Vector2u const& scene_size,
	sf::Vector2f const& tile_size)
	: utils::Tiling<Mode>{tile_size}
	, cells{}
	, scene_size{scene_size}  //, pathfinder{*this}
	, id{id}
	, tileset{tileset} {
	cells.resize(scene_size.x * scene_size.y);
}

template <typename Cell, typename Entity, utils::GridMode Mode>
std::size_t SpatialScene<Cell, Entity, Mode>::getIndex(
	sf::Vector2u const pos) const {
	if (!has(pos)) {
		throw std::out_of_range{
			"scene dimension overflow: " + std::to_string(pos.x) + "," +
			std::to_string(pos.y) + " exceeds size of " +
			std::to_string(scene_size.x) + "x" + std::to_string(scene_size.y)};
	}
	return pos.x + pos.y * scene_size.x;
}

template <typename Cell, typename Entity, utils::GridMode Mode>
bool SpatialScene<Cell, Entity, Mode>::has(sf::Vector2u const& pos) const {
	return pos.x < scene_size.x && pos.y < scene_size.y;
}

template <typename Cell, typename Entity, utils::GridMode Mode>
SpatialCell<Cell, Entity>& SpatialScene<Cell, Entity, Mode>::getCell(
	sf::Vector2u const& pos) {
	auto index = getIndex(pos);
	return cells.at(index);
}

template <typename Cell, typename Entity, utils::GridMode Mode>
SpatialCell<Cell, Entity> const& SpatialScene<Cell, Entity, Mode>::getCell(
	sf::Vector2u const& pos) const {
	auto index = getIndex(pos);
	return cells.at(index);
}

template <typename Cell, typename Entity, utils::GridMode Mode>
template <typename Traverser>
void SpatialScene<Cell, Entity, Mode>::traverse(Traverser& trav) const {
	auto rect = trav.getRange();
	auto size = sf::Vector2i{scene_size};
	
	// adjust to scene bounds
	auto min_x = std::max(rect.left,               0);
	auto min_y = std::max(rect.top,                0);
	auto max_x = std::min(rect.left + rect.width,  size.x - 1);
	auto max_y = std::min(rect.top  + rect.height, size.y - 1);
	
	sf::Vector2u pos;
	for (pos.y = min_y; pos.y <= max_y; ++pos.y) {
		for (pos.x = min_x; pos.x <= max_x; ++pos.x) {
			trav(sf::Vector2f{pos}, getCell(pos).entities);
		}
	}
}

template <typename Cell, typename Entity, utils::GridMode Mode>
sf::Vector2u SpatialScene<Cell, Entity, Mode>::getSize() const {
	return scene_size;
}

} // ::utils

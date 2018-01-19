#include <utils/algorithm.hpp>
#include <utils/math2d.hpp>

namespace utils {

template <typename Cell, typename Entity>
SpatialCell<Cell, Entity>::SpatialCell()
	: Cell{}, entities{} {}

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
void SpatialScene<Cell, Entity, Mode>::query(std::vector<Entity>& entities, sf::FloatRect const & rect) const {
	// get min/max coordinates from rect + applying scene bounds
	auto min_x = static_cast<std::size_t>(std::max(std::floor(rect.left),               0.f));
	auto min_y = static_cast<std::size_t>(std::max(std::floor(rect.top),                0.f));
	auto max_x = static_cast<std::size_t>(std::min(std::ceil (rect.left + rect.width),  scene_size.x - 1.f));
	auto max_y = static_cast<std::size_t>(std::min(std::ceil (rect.top  + rect.height), scene_size.y - 1.f));
	
	// traverse cells
	sf::Vector2u pos;
	for (pos.y = min_y; pos.y <= max_y; ++pos.y) {
		for (pos.x = min_x; pos.x <= max_x; ++pos.x) {
			// append entitiy ids
			auto const & cell = getCell(pos);
			utils::append(entities, cell.entities);
		}
	}
}

template <typename Cell, typename Entity, utils::GridMode Mode>
void SpatialScene<Cell, Entity, Mode>::query(std::vector<Entity>& entities, sf::Vector2f const & center, sf::Vector2f const & size) const {
	return query(entities, {center - size / 2.f, size});
}

template <typename Cell, typename Entity, utils::GridMode Mode>
void SpatialScene<Cell, Entity, Mode>::query(std::vector<Entity>& entities, sf::Vector2f const & center, float radius) const {
	// get min/max coordinates from rect + apply scene bounds
	auto min_x = static_cast<std::size_t>(std::max(std::floor(center.x - radius), 0.f));
	auto min_y = static_cast<std::size_t>(std::max(std::floor(center.y - radius), 0.f));
	auto max_x = static_cast<std::size_t>(std::min(std::ceil (center.x + radius), scene_size.x - 1.f));
	auto max_y = static_cast<std::size_t>(std::min(std::ceil (center.y + radius), scene_size.y - 1.f));
	
	// traverse cells
	sf::Vector2u pos;
	for (pos.y = min_y; pos.y <= max_y; ++pos.y) {
		for (pos.x = min_x; pos.x <= max_x; ++pos.x) {
			// test radius
			auto d = utils::distance(sf::Vector2f{pos}, center);
			if (d <= radius * radius) { // 'cause d is squared
				// append entitiy ids
				auto const & cell = getCell(pos);
				utils::append(entities, cell.entities);
			}
		}
	}
}

template <typename Cell, typename Entity, utils::GridMode Mode>
sf::Vector2u SpatialScene<Cell, Entity, Mode>::getSize() const {
	return scene_size;
}

} // ::utils

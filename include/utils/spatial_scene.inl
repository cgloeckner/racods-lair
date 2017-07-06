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

/*
template <typename Cell, typename Entity, utils::GridMode Mode>
float SpatialScene<Cell, Entity, Mode>::getDistance(sf::Vector2u const & u,
sf::Vector2u const & v) const {
	auto dx = utils::distance(u.x, v.x);
	auto dy = utils::distance(u.y, v.y);
	auto max = std::max(dx, dy);
	auto min = std::min(dx, dy);
	// max-min		: straight distance
	// min * 1.414f	: diagonale distance
	return (max-min) + min * 1.414f;
}
*/

template <typename Cell, typename Entity, utils::GridMode Mode>
sf::Vector2u SpatialScene<Cell, Entity, Mode>::getSize() const {
	return scene_size;
}
}

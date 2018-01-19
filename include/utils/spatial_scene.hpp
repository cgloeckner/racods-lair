#pragma once
#include <cstdint>
#include <vector>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <utils/tiling.hpp>

namespace utils {

using SceneID = std::uint8_t;

template <typename Cell, typename Entity>
struct SpatialCell : public Cell {
	std::vector<Entity> entities;

	SpatialCell();
};

// ---------------------------------------------------------------------------

template <typename Cell, typename Entity, utils::GridMode Mode>
class SpatialScene : public utils::Tiling<Mode> {
  private:
	std::vector<SpatialCell<Cell, Entity>> cells;
	sf::Vector2u const scene_size;

	std::size_t getIndex(sf::Vector2u const pos) const;

  public:
	SpatialScene(SceneID id, sf::Texture const& tileset,
		sf::Vector2u const& scene_size, sf::Vector2f const& tile_size);

	// Pathfinder<SpatialScene<Cell, Entity, Mode>, Entity> pathfinder;
	SceneID const id;
	sf::Texture const& tileset;

	bool has(sf::Vector2u const& pos) const;
	SpatialCell<Cell, Entity>& getCell(sf::Vector2u const& pos);
	SpatialCell<Cell, Entity> const& getCell(sf::Vector2u const& pos) const;

	// float getDistance(sf::Vector2u const & u, sf::Vector2u const & v) const;
	sf::Vector2u getSize() const;
};

}  // ::utils

// include implementation details
#include <utils/spatial_scene.inl>

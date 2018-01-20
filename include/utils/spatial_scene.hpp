#pragma once
#include <cstdint>
#include <vector>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <utils/tiling.hpp>
#include <utils/math2d.hpp>

namespace utils {

using SceneID = std::uint8_t;

template <typename Cell, typename Entity>
struct SpatialCell : public Cell {
	std::vector<Entity> entities;

	SpatialCell();
};

// ---------------------------------------------------------------------------

/// Traversion class for AABB entity query
template <typename Entity>
class AABBEntityQuery {
	private:
		sf::FloatRect range;
		
	public:
		AABBEntityQuery(sf::Vector2f const & center, sf::Vector2f const & size);
		
		sf::IntRect getRange() const;
		void operator()(sf::Vector2f const & pos, std::vector<Entity> const & cell);
		
		std::vector<Entity> entities;
};

// ---------------------------------------------------------------------------

/// Traversion class for circular entity query
template <typename Entity>
class CircEntityQuery {
	private:
		sf::Vector2f center;
		Collider collider;
		
	public:
		CircEntityQuery(sf::Vector2f const & center, float radius);
		
		sf::IntRect getRange() const;
		void operator()(sf::Vector2f const & pos, std::vector<Entity>  const & cell);
		
		std::vector<Entity> entities;
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

	SceneID const id;
	sf::Texture const& tileset;

	bool has(sf::Vector2u const& pos) const;
	SpatialCell<Cell, Entity>& getCell(sf::Vector2u const& pos);
	SpatialCell<Cell, Entity> const& getCell(sf::Vector2u const& pos) const;

	/// Traverse scene's entities using a custom traversion object
	/// using a templated traverser type
	/// @trav Traversion object
	template <typename Traverser>
	void traverse(Traverser& trav) const;

	sf::Vector2u getSize() const;
};

}  // ::utils

// include implementation details
#include <utils/spatial_scene.inl>

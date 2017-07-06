#pragma once
#include <cstdint>
#include <vector>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/VertexArray.hpp>

#include <utils/lighting_system.hpp>

namespace utils {

sf::Image fixTileset(sf::Image const & source, sf::Vector2u const & tilesize);

void scale(sf::Vector2f& vec, sf::Vector2u const & size);
void prepare(sf::Vector2f& tl, sf::Vector2f& tr, sf::Vector2f& br, sf::Vector2f& bl, sf::Vector2u const & offset, sf::Vector2u const & size);

// --------------------------------------------------------------------

using ShadingCase = std::uint8_t;
ShadingCase const ShadeTopLeft = 0x01;
ShadingCase const ShadeTopRight = 0x02;
ShadingCase const ShadeBottomRight = 0x04;
ShadingCase const ShadeBottomLeft = 0x08;

struct OrthoTile {
	bool std_tri;  // standard triangulation
	std::vector<sf::Vertex> vertices;
	std::vector<Edge> edges;  // for lighting calculation

	OrthoTile();

	/// Refresh tile position, texture coordinates and edge-based shading
	/// @pre scale > 0
	/// @pre tile_size > 0
	/// @pre scale.x and scale.y are even
	void refresh(sf::Vector2u const& tile_pos, sf::Vector2u const& scale,
		sf::Vector2u const& offset, sf::Vector2u const& tile_size,
		ShadingCase shading = 0u, bool has_edges = false);

	/// @pre Tile refreshed
	void fetchTile(sf::VertexArray& out) const;

	/// @pre Tile refreshed
	void fetchCollision(sf::Color const& c, sf::VertexArray& out) const;

	/// @pre Tile refreshed
	void fetchGrid(sf::Color const& c, sf::VertexArray& out) const;
};

}  // ::utils

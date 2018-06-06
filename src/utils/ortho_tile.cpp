#include <utils/assert.hpp>
#include <utils/ortho_tile.hpp>

namespace utils {

sf::Vector2f const HalfTilePos{0.5f, 0.5f};

sf::Image fixTileset(sf::Image const & source, sf::Vector2u const & tilesize) {
	// determine new size
	auto size = source.getSize();
	ASSERT(size.x % tilesize.x == 0);
	ASSERT(size.y % tilesize.y == 0);
	auto num_x = size.x / tilesize.x;
	auto num_y = size.y / tilesize.y;
	size.x += num_x * 2;
	size.y += num_y * 2;
	sf::Image atlas;
	atlas.create(size.x, size.y);
	
	// create atlas
	sf::Vector2u offset;
	for (offset.y = 0u; offset.y < num_y; ++offset.y) {
		for (offset.x = 0u; offset.x < num_x; ++offset.x) {
			// copy frame
			auto destX = 1 + offset.x * (tilesize.x + 2);
			auto destY = 1 + offset.y * (tilesize.y + 2);
			sf::IntRect sourceRect;
			sourceRect.left = offset.x * tilesize.x;
			sourceRect.top = offset.y * tilesize.y;
			sourceRect.width = tilesize.x;
			sourceRect.height = tilesize.y;
			atlas.copy(source, destX, destY, sourceRect);
			
			// create left border
			--destX;
			sourceRect.width = 1;
			atlas.copy(source, destX, destY, sourceRect);
			
			// create right border
			destX += tilesize.x + 1;
			sourceRect.left += tilesize.x - 1;
			atlas.copy(source, destX, destY, sourceRect);
			
			// create top border (copying from source to source!)
			destX -= tilesize.x + 1;
			sourceRect.left = destX;
			sourceRect.top = destY;
			sourceRect.width = tilesize.x + 2;
			sourceRect.height = 1;
			--destY;
			atlas.copy(atlas, destX, destY, sourceRect);
			
			// create bottom border (copying from source to source!)
			destY += tilesize.x;
			sourceRect.top = destY;
			++destY;
			atlas.copy(atlas, destX, destY, sourceRect);
		}
	}
	return atlas;
}

void scale(sf::Vector2f& vec, sf::Vector2u const & size) {
	vec.x *= size.x;
	vec.y *= size.y;
}

void prepare(sf::Vector2f& tl, sf::Vector2f& tr, sf::Vector2f& br, sf::Vector2f& bl, sf::Vector2u const & offset, sf::Vector2u const & size) {
	tl = sf::Vector2f(offset.x,			offset.y);
	tr = sf::Vector2f(offset.x + 1.f,	offset.y);
	br = sf::Vector2f(offset.x + 1.f,	offset.y + 1.f);
	bl = sf::Vector2f(offset.x,			offset.y + 1.f);
	scale(tl, size);
	scale(tr, size);
	scale(br, size);
	scale(bl, size);
}

// --------------------------------------------------------------------

OrthoTile::OrthoTile() : std_tri{true}, vertices{}, edges{} {}

void OrthoTile::refresh(sf::Vector2u const& tile_pos,
	sf::Vector2u const& scale, sf::Vector2u const& offset,
	sf::Vector2u const& tile_size, ShadingCase shading, bool has_edges) {
	ASSERT(scale.x > 0u);
	ASSERT(scale.y > 0u);
	ASSERT(scale.x % 2 == 0);
	ASSERT(scale.y % 2 == 0);
	ASSERT(tile_size.x > 0u);
	ASSERT(tile_size.y > 0u);
	vertices.clear();
	edges.clear();

	// build tiles
	vertices.resize(4u);
	prepare(vertices[0].position, vertices[1].position, vertices[2].position,
		vertices[3].position, tile_pos, scale);
	prepare(vertices[0].texCoords, vertices[1].texCoords, vertices[2].texCoords,
		vertices[3].texCoords, offset, tile_size);
	
	sf::Vector2f delta{1.f, 1.f};
	delta.x += offset.x * 2;
	delta.y += offset.y * 2;
	
	for (std::size_t i = 0u; i < 4u; ++i) {
		// fix tex coords to suit the modified atlas
		vertices[i].texCoords += delta;
	}
	
	if (has_edges) {
		// build edges (for shadow casting)
		edges.resize(4u);
		edges[0].u = vertices[0].position;
		edges[0].v = vertices[1].position;
		edges[1].u = vertices[1].position;
		edges[1].v = vertices[2].position;
		edges[2].u = vertices[2].position;
		edges[2].v = vertices[3].position;
		edges[3].u = vertices[3].position;
		edges[3].v = vertices[0].position;
	}

	if (shading > 0u) {
		// colorize vertices for linear shading to void tiles
		for (auto i = 0u; i < 4; ++i) {
			vertices[i].color = sf::Color::White;
		}
		std_tri = true;
		if (shading == (ShadeTopLeft | ShadeTopRight | ShadeBottomLeft)) {
			// use alternate triangulation of only BottomRight is unshaded
			std_tri = false;
		}
		if (shading == (ShadeTopRight | ShadeBottomRight | ShadeBottomLeft)) {
			// use alternate triangulation of only TopLeft is unshaded
			std_tri = false;
		}
		if (shading == ShadeBottomRight) {
			// use alternate triangulation of only BottomRight is shaded
			std_tri = false;
		}
		if (shading == ShadeTopLeft) {
			// use alternate triangulation of only TopLeft is shaded
			std_tri = false;
		}
		if (shading & ShadeTopLeft) {
			vertices[0].color = sf::Color::Black;
		}
		if (shading & ShadeTopRight) {
			vertices[1].color = sf::Color::Black;
		}
		if (shading & ShadeBottomRight) {
			vertices[2].color = sf::Color::Black;
		}
		if (shading & ShadeBottomLeft) {
			vertices[3].color = sf::Color::Black;
		}
	}
}

void OrthoTile::fetchTile(sf::VertexArray& out) const {
	ASSERT(vertices.size() == 4u);
	if (std_tri) {
		out.append(vertices[0]);
		out.append(vertices[1]);
		out.append(vertices[2]);
		out.append(vertices[0]);
		out.append(vertices[2]);
		out.append(vertices[3]);
	} else {
		out.append(vertices[0]);
		out.append(vertices[1]);
		out.append(vertices[3]);
		out.append(vertices[1]);
		out.append(vertices[2]);
		out.append(vertices[3]);
	}
}

void OrthoTile::fetchCollision(sf::Color const& c, sf::VertexArray& out) const {
	ASSERT(vertices.size() == 4u);
	for (auto i : {0u, 1u, 2u, 0u, 2u, 3u}) {
		out.append({vertices[i].position, c});
	}
}

void OrthoTile::fetchGrid(sf::Color const& c, sf::VertexArray& out) const {
	ASSERT(vertices.size() == 4u);
	out.append({vertices[0].position, c});
	out.append({vertices[1].position, c});
	out.append({vertices[1].position, c});
	out.append({vertices[2].position, c});
	out.append({vertices[2].position, c});
	out.append({vertices[3].position, c});
	out.append({vertices[3].position, c});
	out.append({vertices[0].position, c});
}

}  // ::utils

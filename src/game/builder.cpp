#include <algorithm>
#include <Thor/Math.hpp>

#include <utils/math2d.hpp>
#include <game/builder.hpp>

namespace game {

namespace dungeon_impl {

bool shouldBeWall(core::Dungeon const& dungeon, sf::Vector2u const& pos) {
	auto const& cell = dungeon.getCell(pos);
	if (cell.terrain != core::Terrain::Void) {
		// wall can only be placed at void!
		return false;
	}

	sf::Vector2i delta;
	for (delta.y = -1; delta.y <= 1; ++delta.y) {
		for (delta.x = -1; delta.x <= 1; ++delta.x) {
			if (delta.x == 0 && delta.y == 0) {
				continue;
			}
			auto tmp = sf::Vector2u{sf::Vector2i{pos} + delta};
			if (!dungeon.has(tmp)) {
				continue;
			}
			auto const& cell = dungeon.getCell(tmp);
			if (cell.terrain == core::Terrain::Floor) {
				// found near by floor!
				return true;
			}
		}
	}
	return false;
}

bool shouldBeShaded(core::Dungeon const& dungeon, sf::Vector2u const& pos,
	sf::Vector2i const& delta) {
	auto tmp = sf::Vector2u{sf::Vector2i{pos} + delta};
	if (!dungeon.has(tmp)) {
		return true;
	}
	auto const& cell = dungeon.getCell(tmp);
	return cell.terrain == core::Terrain::Void;
}

utils::ShadingCase getShadingCase(
	core::Dungeon const& dungeon, sf::Vector2u const& pos) {
	utils::ShadingCase shading_case = 0u;
	sf::Vector2i dir;
	for (dir.y = -1; dir.y <= 1; ++dir.y) {
		for (dir.x = -1; dir.x <= 1; ++dir.x) {
			if (dir.x == 0 && dir.y == 0) {
				continue;
			}
			auto tmp = sf::Vector2u{sf::Vector2i{pos} + dir};
			if (!dungeon.has(tmp) ||
				dungeon.getCell(tmp).terrain == core::Terrain::Void) {
				// either neigbor is out of grid or explicitly void tile
				if (dir.y == -1) {
					if (dir.x == -1) {
						shading_case |= utils::ShadeTopLeft;
					} else if (dir.x == 1) {
						shading_case |= utils::ShadeTopRight;
					} else if (dir.x == 0) {
						shading_case |=
							utils::ShadeTopLeft | utils::ShadeTopRight;
					}
				} else if (dir.y == 1) {
					if (dir.x == -1) {
						shading_case |= utils::ShadeBottomLeft;
					} else if (dir.x == 1) {
						shading_case |= utils::ShadeBottomRight;
					} else if (dir.x == 0) {
						shading_case |=
							utils::ShadeBottomLeft | utils::ShadeBottomRight;
					}
				} else if (dir.y == 0) {
					if (dir.x == -1) {
						shading_case |=
							utils::ShadeTopLeft | utils::ShadeBottomLeft;
					} else if (dir.x == 1) {
						shading_case |=
							utils::ShadeTopRight | utils::ShadeBottomRight;
					}
				}
			}
		}
	}
	return shading_case;
}

void placeFloor(core::Dungeon& dungeon, sf::Vector2u const& pos) {
	auto& cell = dungeon.getCell(pos);
	cell.terrain = core::Terrain::Floor;
}

void placeWall(core::Dungeon& dungeon, sf::Vector2u const& pos) {
	auto& cell = dungeon.getCell(pos);
	cell.terrain = core::Terrain::Wall;
}

void prepareTile(rpg::TilesetTemplate const& tileset, core::Dungeon& dungeon,
	sf::Vector2u const& pos) {
	auto& cell = dungeon.getCell(pos);
	sf::Vector2u offset;
	utils::ShadingCase shade = 0u;
	bool has_edges = false;
	if (cell.terrain == core::Terrain::Wall) {
		// prepare wall
		auto index = thor::random(0u, tileset.walls.size() - 1u);
		offset = tileset.walls[index];
		shade = getShadingCase(dungeon, pos);
		has_edges = true;

	} else if (cell.terrain == core::Terrain::Floor) {
		// prepare floor
		auto index = thor::random(0u, tileset.floors.size() - 1u);
		offset = tileset.floors[index];

	}

	// prepare tile
	cell.tile.refresh(pos, tileset.tilesize, offset, tileset.tilesize, shade, has_edges);
}

void makeTransparent(core::Dungeon& dungeon, sf::Vector2u const & pos, bool transparent) {
	for (auto& v: dungeon.getCell(pos).tile.vertices) {
		if (transparent) {
			v.color.a = 200u;
		} else {
			v.color.a = 255u;
		}
	}
}

// --------------------------------------------------------------------

void flipX(sf::Vector2u& pos, unsigned int max_value) {
	pos.x = max_value - 1 - pos.x;
}

void flipY(sf::Vector2u& pos, unsigned int max_value) {
	pos.y = max_value - 1 - pos.y;
}

void transform(sf::Vector2u& pos, unsigned int max_value, float angle, bool flip_x, bool flip_y) {
	if (angle == 90.f) {
		std::swap(pos.x, pos.y);
		flipX(pos, max_value);
	} else if (angle == 180.f) {
		flipX(pos, max_value);
		flipY(pos, max_value);
	} else if (angle == 270.f) {
		std::swap(pos.x, pos.y);
		flipY(pos, max_value);
	}
	if (flip_x) {
		flipX(pos, max_value);
	}
	if (flip_y) {
		flipY(pos, max_value);
	}
}

void transform(sf::Vector2i& dir, float angle, bool flip_x, bool flip_y) {
	while (angle >= 90.f) {
		std::swap(dir.x, dir.y);
		dir.x *= -1;
		angle -= 90.f;
	}
	if (flip_x) {
		dir.x *= -1;
	}
	if (flip_y) {
		dir.y *= -1;
	}
}

}  // ::dungeon_impl

// ---------------------------------------------------------------------------

BuildSettings::BuildSettings()
	: cell_size{30u}
	, path_width{3u}
	, random_transform{true}
	, editor_mode{false} {
}

// ---------------------------------------------------------------------------

RoomBuilder::RoomBuilder(unsigned int left, unsigned int top,
	RoomTemplate const & tpl)
	: offset{left, top}
	, tpl{tpl}
	, angle{0.f}
	, flip_x{false}
	, flip_y{false} {
}

BuildInformation::Floors RoomBuilder::operator()(core::Dungeon& dungeon,
	BuildSettings const & settings) const {
	BuildInformation::Floors floors;
	floors.reserve(tpl.cells.size());
	
	for (auto const & pair: tpl.cells) {
		auto pos = pair.first;
		dungeon_impl::transform(pos, settings.cell_size, angle, flip_x, flip_y);
		pos += offset;
		if (!pair.second.wall) {
			dungeon_impl::placeFloor(dungeon, pos);
			floors.push_back(pos);
		} else {
			dungeon_impl::placeWall(dungeon, pos);
		}
	}
	
	return floors;
}

bool RoomBuilder::isValid(sf::Vector2u const& grid_size) const {
	unsigned int width{0u}, height{0u};
	for (auto const & pair: tpl.cells) {
		width = std::max(width, pair.first.x+1u);
		height = std::max(height, pair.first.y+1u);
	}
	return (offset.x + width < grid_size.x) && (offset.y + height < grid_size.y);
}

// ---------------------------------------------------------------------------

PathBuilder::PathBuilder(
	unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2)
	: origin{x1, y1}, target{x2, y2} {}

PathBuilder::PathBuilder(sf::Vector2u const& origin, sf::Vector2u const& target)
	: origin{origin}, target{target} {}

BuildInformation::Floors PathBuilder::operator()(core::Dungeon& dungeon, BuildSettings const & settings) const {
	BuildInformation::Floors floors;
	
	// prepare traversal
	auto const dx = utils::distance(origin.x, target.x);
	auto const dy = utils::distance(origin.y, target.y);
	floors.reserve(2u * settings.path_width * std::max(dx, dy));
	
	int const stepx = (origin.x <= target.x) ? 1 : -1;
	int const stepy = (origin.y <= target.y) ? 1 : -1;
	auto const step = settings.path_width / 2u;
	
	// populate dungeon
	for (auto i = 0; i < settings.path_width; ++i) {
		for (auto j = 0; j < settings.path_width; ++j) {
			for (int x = 0; x <= dx; ++x) {
				// place floor
				sf::Vector2u pos{origin.x + x * stepx, origin.y};
				pos.x = pos.x + i - step;
				pos.y = pos.y + j - step;
				dungeon_impl::placeFloor(dungeon, pos);
				floors.push_back(pos);
			}
			for (int y = 0; y <= dy; ++y) {
				// place floor
				// note: "+ dx * stepx" to respect horizontal delta
				sf::Vector2u pos{origin.x + dx * stepx, origin.y + y * stepy};
				pos.x = pos.x + i - step;
				pos.y = pos.y + j - step;
				dungeon_impl::placeFloor(dungeon, pos);
				floors.push_back(pos);
			}
		}
	}
	
	return floors;
}

bool PathBuilder::isValid(
	sf::Vector2u const& grid_size, unsigned int width) const {
	return (origin.x >= 1) && (origin.y >= 1) &&
		   (origin.x + width < grid_size.x) &&
		   (origin.y + width < grid_size.y) && (target.x >= 1) &&
		   (target.y >= 1) && (target.x + width < grid_size.x) &&
		   (target.y + width < grid_size.y);
}

// ---------------------------------------------------------------------------

DungeonBuilder::DungeonBuilder(sf::Vector2u const& grid_size)
	: grid_size{grid_size}
	, rooms{}
	, paths{}
	, info{} {
}

void DungeonBuilder::operator()(rpg::TilesetTemplate const& tileset,
	core::Dungeon& dungeon, BuildSettings const & settings) {
	info = BuildInformation{};
	ASSERT(grid_size == dungeon.getSize());
	
	// place all paths' floor tiles
	for (auto const& path : paths) {
		if (!settings.editor_mode) {
			ASSERT(path.isValid(grid_size, settings.path_width));
		}
		info.corridors.push_back(path(dungeon, settings));
	}
	
	// place all rooms' floor and (forced) wall tiles
	for (auto const & room : rooms) {
		ASSERT(room.isValid(grid_size));
		info.rooms.push_back(room(dungeon, settings));
	}
	
	// place bordering walls
	sf::Vector2u pos;
	std::vector<sf::Vector2u> autowalls;
	autowalls.reserve(5u * (grid_size.x + grid_size.x));
	for (pos.y = 0u; pos.y < grid_size.y; ++pos.y) {
		for (pos.x = 0u; pos.x < grid_size.x; ++pos.x) {
			if (dungeon_impl::shouldBeWall(dungeon, pos)) {
				dungeon_impl::placeWall(dungeon, pos);
				autowalls.push_back(pos);
			}
		}
	}

	// apply wall shading
	for (pos.y = 0u; pos.y < grid_size.y; ++pos.y) {
		for (pos.x = 0u; pos.x < grid_size.x; ++pos.x) {
			dungeon_impl::prepareTile(tileset, dungeon, pos);
		}
	}
	
	if (settings.editor_mode) {
		// make corridors semi-transparent
		for (auto const & corridor: info.corridors) {
			for (auto const & pos: corridor) {
				dungeon_impl::makeTransparent(dungeon, pos);
			}
		}
		// make room's floor non-transparent
		for (auto const & room: info.rooms) {
			for (auto const & pos: room) {
				dungeon_impl::makeTransparent(dungeon, pos, false);
			}
		}
		// make auto-walls semi-transparent
		for (auto const & pos: autowalls) {
			dungeon_impl::makeTransparent(dungeon, pos);
		}
	}
}

}  // ::rage

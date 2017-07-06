#pragma once
#include <vector>

#include <core/dungeon.hpp>
#include <rpg/resources.hpp>
#include <game/resources.hpp>

namespace game {

namespace dungeon_impl {

/// Returns true if the specified tile is pure void
/**
 *	@param dungeon DungeonBuilder to query at
 *	@param pos Position to query
 *	@return true if void tile
 */
bool shouldBeWall(core::Dungeon const& dungeon, sf::Vector2u const& pos);

/// Returns true if the specified neighbor is pure void
/**
 *	@param dungeon DungeonBuilder to query at
 *	@param pos Position used as origin
 *	@param delta Direction that specifies the neighbor
 *	@return true if void tile or tile does not exist
 */
bool shouldBeShaded(core::Dungeon const& dungeon, sf::Vector2u const& pos,
	sf::Vector2i const& delta);

/// Returns shading case for the specified position
/**
 *	The shading case specifies whether a wall tile is partially shaded to
 *	create a gradient effect to void tiles. If zero is returned, the tile
 *	is not shaded.
 *
 *	@param dungeon DungeonBuilder to query at
 *	@param pos Position that should be shaded
 *	@return shading case
 */
utils::ShadingCase getShadingCase(
	core::Dungeon const& dungeon, sf::Vector2u const& pos);

/// Place a floor tile
/**
 *	This will place a floor tile at the specified position.
 *
 *	@param dungeon DungeonBuilder to set at
 *	@param pos Position to set at
 */
void placeFloor(core::Dungeon& dungeon, sf::Vector2u const& pos);

/// Place a wall tile
/**
 *	This will place a wall tile at the specified position.
 *
 *	@param dungeon DungeonBuilder to set at
 *	@param pos Position to set at
 */
void placeWall(core::Dungeon& dungeon, sf::Vector2u const& pos);

/// Prepare Tile for rendering
/**
 *	This will prepare the tile for rendering and apply shading if necessary.
 *	A random tile is chosen from the tileset.
 *
 *	@param tileset Tileset to ue for tile seleciton
 *	@param dungeon DungeonBuilder to prepare at
 *	@param pos Position to prepare at
 */
void prepareTile(rpg::TilesetTemplate const& tileset, core::Dungeon& dungeon,
	sf::Vector2u const& pos);

void makeTransparent(core::Dungeon& dungeon, sf::Vector2u const & pos, bool transparent=true);

/// Flip position
void flipX(sf::Vector2u& pos, unsigned int max_value);
void flipY(sf::Vector2u& pos, unsigned int max_value);

/// Transform position
void transform(sf::Vector2u& pos, unsigned int max_value, float angle, 
	bool flip_x, bool flip_y);

/// Transform direction
void transform(sf::Vector2i& dir, float angle, bool flip_x, bool flip_y);

}  // ::dungeon_impl

// ---------------------------------------------------------------------------

struct BuildSettings {
	unsigned int cell_size, path_width;
	bool random_transform, editor_mode;
	
	BuildSettings();
};

// ---------------------------------------------------------------------------

struct BuildInformation {
	using Floors = std::vector<sf::Vector2u>;
	std::vector<Floors> rooms, corridors;
};

// ---------------------------------------------------------------------------

struct RoomBuilder {
	sf::Vector2u offset;
	RoomTemplate const & tpl;

	float angle;
	bool flip_x, flip_y;

	RoomBuilder(unsigned int left, unsigned int top, RoomTemplate const & tpl);

	/// Dig a room inside the dungeon
	/// This will dig the specified room inside the dungeon. The room's
	/// borders are not set here. Its inner is made of floor. The room needs
	/// at least size 3x3 to cause a 1x1 floor.
	/// @pre width > 3
	/// @pre height > 3
	/// @pre rotation in {0, 90, 180, 270}
	/// @param dungeon DungeonBuilder to populate
	/// @param settings BuildSettings to use
	/// @param rotation Angle for rotation
	/// @param flipX Flip x-coordinates
	/// @param flipY Flip y-coordinates
	/// @return addition to dungeon's built information
	BuildInformation::Floors operator()(core::Dungeon& dungeon,
		BuildSettings const & settings) const;

	/// Checks whether the room can be placed inside a dungeon of the given size
	/**
	 *	Conditions, because the room's outline will be made of wall:
	 *	- left >= 0u (provided by unsigned int)
	 *	- top >= 0 (provided by unsigned int)
	 *	- left + width < grid.size.x
	 *	- top + height < grid_size.y
	 *
	 *	@param grid_size Size of the dungeon
	 *	@return true if room can be placed
	 */
	bool isValid(sf::Vector2u const& grid_size) const;
};

// ---------------------------------------------------------------------------

struct PathBuilder {
	sf::Vector2u origin, target;

	PathBuilder(
		unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);
	PathBuilder(sf::Vector2u const& origin, sf::Vector2u const& target);

	/// Dig a path inside the dungeon
	/**
	 *	This will dig the specified path inside the dungeon. The path's real
	 *	width is specified. No borders are set, only floor tiles are placed.
	 *	Neither origin nor target not are allowed to intersect with the
	 *	dungeon's borders (with respect to the path's width)
	 *
	 *	@param dungeon DungeonBuilder to populate
	 *	@param settings BuildSettings to use
	/// @return addition to dungeon's build information
	 */
	BuildInformation::Floors operator()(core::Dungeon& dungeon, BuildSettings const & settings) const;

	/// Checks whether the path can be placed inside a dungeon of the given size
	/**
	 *	Conditions per position (origin, target) due to the wall-made border
	 *	around the path:
	 *	- x >= 1
	 *	- y >= 1
	 *	- x + width < grid_size.x
	 *	- y + height < grid_size.y
	 *
	 *	@param grid_size Size of the dungeon
	 *	@param width Width of the path
	 *	@return true if path can be placed
	 */
	bool isValid(sf::Vector2u const& grid_size, unsigned int width) const;
};

// ---------------------------------------------------------------------------

struct DungeonBuilder {
	sf::Vector2u grid_size;
	std::vector<RoomBuilder> rooms;
	std::vector<PathBuilder> paths;

	BuildInformation info; // populated after building

	DungeonBuilder(sf::Vector2u const& grid_size);

	/// Dig all rooms and paths inside the dungeon
	/// This will dig all rooms and paths inside the dungeon. First all floor
	/// tiles are placed. After that a thin border of wall tiles is placed
	/// next to each bordering floor tile.
	/// @pre grid_size == dungeon size
	/// @pre All rooms are valid referring to the dungeon's size
	/// @pre All paths are valid referring to the dungeon's size and path's
	/// width
	/// @param tileset TilesetTemplate to use
	/// @param dungeon DungeonBuilder to populate
	/// @param settings BuildSettings to use
	/// @return dungeon's build information
	void operator()(rpg::TilesetTemplate const& tileset,
		core::Dungeon& dungeon, BuildSettings const & settings);
};

}  // ::rage

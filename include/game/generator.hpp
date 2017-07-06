#pragma once
#include <vector>
#include <memory>
#include <SFML/System/Vector2.hpp>

#include <game/builder.hpp>
#include <game/navigator.hpp>
#include <game/resources.hpp>

namespace game {

/// Dungeon generation data
/**
 *	builder describes the narrow layout of the dungeon,
 *	graph describes the broad layout of the dungeon
 */
struct DungeonData {
	DungeonBuilder builder;
	DungeonGraph graph;

	/// Create new dungeon data
	/**
	 *	Initialize builder and graph using the given arguments
	 *
	 *	@param grid_size Total size of the dungeon
	 *	@param layout_size Size of the dungeon's graph layout
	 */
	DungeonData(sf::Vector2u const& grid_size, sf::Vector2u const& layout_size);
};

// ---------------------------------------------------------------------------

class DungeonGenerator {
  private:
	core::LogContext& log;
	std::vector<std::unique_ptr<DungeonData>> data;
	
  public:
	GeneratorSettings settings;
	std::vector<RoomTemplate const*> rooms;
	
	DungeonGenerator(core::LogContext& log);
	
	void layoutifySize(sf::Vector2u& grid_size);
	
	/// Generate dungeon data
	/// This generates dungeon data for the given settings. All
	/// randomization is done here.
	/// @pre !all_rooms.empty()
	/// @pre each room template is valid
	/// @param grid_size Total size used for the dungeon
	/// @return dungeon data
	DungeonData& generate(utils::SceneID id, sf::Vector2u grid_size);
	
	DungeonData& operator[](utils::SceneID id);
	DungeonData const & operator[](utils::SceneID id) const;
	
	void clear();
};

} // ::game


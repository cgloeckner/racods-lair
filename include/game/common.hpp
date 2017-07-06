#pragma once
#include <vector>
#include <memory>
#include <mutex>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

#include <utils/spatial_scene.hpp>
#include <core/common.hpp>

namespace game {

/// fading delay (ms) before object release
unsigned int const FADE_DELAY = 1000;

// used for pathfinding
ENUM(PathPhase, Broad, (Broad)(Narrow))

using Path = std::vector<sf::Vector2u>;

/// Interface of Pathfinding API
/**
 *	This interface is used to give several subsystems access to the
 *	pathfinding API.
 *
struct PathInterface {
	/// Schedule a pathfinding request
	**
	 *	The given pathfinding request is scheduled and calculated as soon as
	 *	possible. The returned shared pointer can be used to query the
	 *	calculation's result. Once the pointer is not null anymore, the
	 *	calculation has finished and the path can be accessed. Due to the
	 *	shared ownership, the pointer can be dropped once the path is not
	 *	needed anymore.
	 *
	 *	@param phase Determines whether broadphase or narrowphase are used
	 *	@param actor Actor's object ID
	 *	@param scene Scene ID
	 *	@param source Beginning of the path
	 *	@param target End of the path
	 *	@return shared ponter to calculated path.
	 *
	virtual std::shared_ptr<Path const> schedule(PathPhase phase, core::ObjectID
actor, utils::SceneID scene, sf::Vector2u const & source, sf::Vector2u const &
target) = 0;
};
*/

}  // ::rage

ENUM_STREAM(game::PathPhase)

SET_ENUM_LIMITS(game::PathPhase::Broad, game::PathPhase::Narrow)

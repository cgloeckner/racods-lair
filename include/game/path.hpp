#pragma once
#include <list>
#include <future>
#include <SFML/System/Time.hpp>

#include <utils/pathfinder.hpp>
#include <game/common.hpp>
#include <game/navigator.hpp>

namespace game {

namespace path_impl {

/// Maximum length of a path used within pathfinding
extern std::size_t const MAX_PATH_LENGTH;

/// Combines several data for a pathfinding request
struct Request {
	core::ObjectID actor;
	utils::SceneID scene;
	sf::Vector2u source, target;
	std::promise<Path> path;

	Request();
};

}  // ::path_impl

// ---------------------------------------------------------------------------

/// Handles pathfinding requests without blocking
/**
 *	It calculates some pathfinding steps until the maximum frame time
 *	was exceeded or all calculations are done. So the path calculation
 *	itself does not block until its done. Furthermore the pathfinding
 *	can be distributed to multiple frames.
 */
class PathSystem {

  private:
	core::LogContext& log;
	// Registered scene navigators
	std::vector<Navigator*> scenes;
	// Pending requests
	std::list<path_impl::Request> requests;

  public:
	/// Create system with maximum frame time
	PathSystem(core::LogContext& log);

	/// Register a scene's navigator
	/**
	 *	Each scene needs to be registered using its ID and the
	 *	corresponding navigator, that provides broadphase and
	 *	narrowphase pathfinding objects.
	 *
	 *	@param id Scene ID of the dungeon
	 *	@param navigator Reference to the dungeon's navigator
	 */
	void addScene(utils::SceneID id, Navigator& navigator);

	/// Schedule a new pathfinding request
	/**
	 *	A new request is added to the list of pending requests.
	 *	Once it was calculated, the returned future is
	 *	automatically populated with the resulting path. If no path
	 *	was found, the path only
	 *	contains the source position.
	 *
	 *	@param actor Actor's object ID
	 *	@param scene Dungeon's scene ID
	 *	@param source Source position in world scale
	 *	@param target Target position in world scale
	 *	@return future to path
	 */
	virtual std::future<Path> schedule(core::ObjectID actor,
		utils::SceneID scene, sf::Vector2u const& source,
		sf::Vector2u const& target);

	/// Perform calculations
	/**
	 *	Will calculate as many calculations as possible until
	 *	either the maximum frame time exceeded or all requests
	 *	are handled.
	 *
	 *	@param max_elapse Maximum frame time to exceed
	 *	@return number of finished calculations
	 */
	std::size_t calculate(sf::Time const& max_elapse);
};

}  // ::rage

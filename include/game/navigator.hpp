#pragma once
#include <vector>
#include <memory>
#include <SFML/System/Vector2.hpp>

#include <utils/algorithm.hpp>
#include <utils/pathfinder.hpp>
#include <core/collision.hpp>
#include <core/dungeon.hpp>
#include <core/entity.hpp>
#include <game/builder.hpp>

namespace game {

namespace navigator_impl {

/// Calculate Euclidian-like distance using a discrete grid
/**
 *	The distance is based on two rules: Each straight move has distance
 *	1.0, each diagonal move has distance 1.414 ~= sqrt(2). The function
 *	determines how many straight and diagonal steps are inbetween both
 *	positions and returns the total distance using both rules.
 *
 *	@param u First vector
 *	@param v Second vector
 *	@param Final distance
 */
float distance(sf::Vector2u const& u, sf::Vector2u const& v);

}  // ::navigator_impl

// ---------------------------------------------------------------------------

/// Used at the dungeon graph
/**
 *	Each node relates to the dungeon as a graph. Nodes can be connected
 *	by `paths`, where each node knows all connected nodes. Furthermore,
 *	each node has an offset position, which refers to the node's grid
 *	position. This graph can be used for broadphase pathfinding.
 */
struct NavigationNode {
	sf::Vector2u offset;
	std::vector<NavigationNode*> paths;

	/// Create node for given offset
	/**
	 *	The given offset does not map to world coordinates; it is
	 *	related to the graph's layout.
	 *
	 *	@param offset Grid position of the node
	 */
	NavigationNode(sf::Vector2u const& offset);
};

// ---------------------------------------------------------------------------

/// Used to create a graph from the dungeon layout
struct DungeonGraph {
  private:
	sf::Vector2u const size;
	std::vector<std::unique_ptr<NavigationNode>> nodes;

  public:
	/// Create an empty graph
	/**
	 *	@param size Size of the layout used for the graph
	 */
	DungeonGraph(sf::Vector2u const& size);

	/// Move construct dungeon graph
	DungeonGraph(DungeonGraph&& other);

	/// Add a new node to the graph
	/**
	 *	The new node will be placed at the given cell position.
	 *	This position is related to the layout, not to world
	 *	coordinates.
	 *
	 *	@pre cell_pos < size (per coordinate)
	 *	@param cell_pos Cell position of the node
	 */
	void addNode(sf::Vector2u const& cell_pos);

	/// Add a new path between two existing nodes
	/**
	 *	The new path will connect two nodes, which are determined
	 *	by the both cell positions.
	 *
	 *	@pre src, dst < size (per coordinate)
	 *	@pre src, dst refer to existing nodes
	 *	@param src Cell position of the first node
	 *	@param dst Cell position of the second node
	 */
	void addPath(sf::Vector2u const& src, sf::Vector2u const& dst);

	/// Query the node
	/**
	 *	The returned pointer refers to the node, which is located
	 *	at the given cell position (or equals nullptr).
	 *
	 *	@pre cell_pos < size (per coordinate)
	 *	@param cell_pos Cell position of the node
	 *	@return pointer to the node or nullptr
	 */
	NavigationNode* getNode(sf::Vector2u const& cell_pos);

	/// Query the node
	/**
	 *	const version of `getNode()`
	 */
	NavigationNode const* getNode(sf::Vector2u const& cell_pos) const;

	/// Calculate Euclidian-like distance using a discrete grid
	/**
	 *	The distance is based on two rules: Each straight move has distance
	 *	1.0, each diagonal move has distance 1.414 ~= sqrt(2). The function
	 *	determines how many straight and diagonal steps are inbetween both
	 *	positions and returns the total distance using both rules.
	 *
	 *	@param u First vector
	 *	@param v Second vector
	 *	@param Final distance
	 */
	float getDistance(sf::Vector2u const& u, sf::Vector2u const& v) const;

	/// Query layout size
	/**
	 *	@return layout size of the grid
	 */
	sf::Vector2u getSize() const;

	/// Query accessable nodes
	/**
	 *	Queries accessable nodes for the given actor and the
	 *	provided cell position.
	 *
	 *	@param actor Actor's object ID
	 *	@param pos Current position
	 *	@param ignore Vector of Entities to ignore
	 *	@return array of neighbor positions
	 */
	std::vector<sf::Vector2u> getNeighbors(
		core::ObjectID actor, sf::Vector2u const& pos,
		std::vector<core::ObjectID> const& ignore={}) const;
};

// ---------------------------------------------------------------------------

/// Used for narrow-phase pathfinding
/**
 *	When using this for pathfinding, collision information are
 *	considered, too.
 */
struct NavigationScene {
  private:
	core::MovementManager const & movement;
	core::CollisionManager const& collision;
	core::Dungeon const& dungeon;
	
	mutable core::CollisionResult coll_result;

  public:
	/// Create navigation scene for a specific dungeon
	/// The creation is based on a collision manager and a specific
	/// dungeon that are used.
	/// @param movement Const reference to movement manager
	/// @param collision Const reference to collision manager
	/// @param dungeon Const reference to dungeon
	NavigationScene(core::MovementManager const & movement,
		core::CollisionManager const& collision, core::Dungeon const& dungeon);

	/// Calculate Euclidian-like distance using a discrete grid
	/**
	 *	The distance is based on two rules: Each straight move has distance
	 *	1.0, each diagonal move has distance 1.414 ~= sqrt(2). The function
	 *	determines how many straight and diagonal steps are inbetween both
	 *	positions and returns the total distance using both rules.
	 *
	 *	@param u First vector
	 *	@param v Second vector
	 *	@param Final distance
	 */
	float getDistance(sf::Vector2u const& u, sf::Vector2u const& v) const;

	/// Query layout size
	/**
	 *	@return layout size of the grid
	 */
	sf::Vector2u getSize() const;

	/// Query accessable nodes
	/**
	 *	Queries accessable nodes for the given actor and the
	 *	provided cell position. Object collision is ignored here.
	 *
	 *	@param actor Actor's object ID
	 *	@param pos Current position
	 *	@param ignore Vector of Entities to ignore
	 *	@return array of neighbor positions
	 */
	std::vector<sf::Vector2u> getNeighbors(
		core::ObjectID actor, sf::Vector2u const& pos,
		std::vector<core::ObjectID> const& ignore={}) const;

	/// Query whether can access node
	/**
	 *	Query whether the given actor can access the given position.
	 *	Object collision is ignored here.
	 *
	 *	@param actor Actor's object ID
	 *	@param pos Current position
	 *	@return true if possible
	 */
	bool canAccess(core::ObjectID actor, sf::Vector2u const& pos) const;
};

// ---------------------------------------------------------------------------

/// Actual pathfinding object
/**
 *	The navigator holds to pathfinder objects, based on the graph and
 *	the scene. Those are used for broadphase and narrowphase
 *	pathfinding.
 */
struct Navigator {
	DungeonGraph graph;
	NavigationScene scene;
	/// @note broadphase not fully implemented (c
	//utils::Pathfinder<DungeonGraph, core::ObjectID> broadphase;
	utils::Pathfinder<NavigationScene, core::ObjectID> narrowphase;

	/// Create navigator based on a specific grid and scene
	/**
	 *	@param grid Rvalue reference to dungeon graph
	 *	@param scene Rvalue reference to navigation scene
	 */
	Navigator(DungeonGraph&& graph, NavigationScene&& scene);
};

// ---------------------------------------------------------------------------

class NavigationSystem {
  private:
	std::vector<std::unique_ptr<Navigator>> navis;

  public:
	NavigationSystem();

	Navigator& create(utils::SceneID id, core::MovementManager const & movement,
		core::CollisionManager const& collision, core::Dungeon const& dungeon,
		DungeonBuilder const& builder);

	Navigator& operator[](utils::SceneID id);
	
	void clear();
};

}  // ::rage

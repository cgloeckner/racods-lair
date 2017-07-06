#pragma once
#include <vector>
#include <unordered_set>
#include <SFML/System/Vector2.hpp>

//#include <utils/priority_queue.hpp>

namespace utils {

using Path = std::vector<sf::Vector2u>;

/// Describes an A*-Node
struct PathNode {
	sf::Vector2u pos;
	PathNode const* previous;
	float f, g;

	PathNode(sf::Vector2u pos = {0u, 0u});
};

/// Compare helper
struct OpenlistCondition {
	bool operator()(PathNode const& lhs, PathNode const& rhs) const;
};

/// Hash helper
struct ClosedlistHasher {
	std::size_t operator()(PathNode const& n) const;
};

/// Equal helper
struct ClosedlistEqual {
	bool operator()(PathNode const& lhs, PathNode const& rhs) const;
};

// --------------------------------------------------------------------

/// This deals with path calculations for a single scene
template <typename Scene, typename Entity>
class Pathfinder {
	private:
		Scene const& scene;
		OpenlistCondition compare;
		
		// typical A*-related containers
		std::vector<PathNode> openlist;
		std::unordered_set<PathNode, ClosedlistHasher, ClosedlistEqual> closedlist;
		
		// request data
		Entity entity_id;
		sf::Vector2u origin, target;
		std::size_t max_length;
		
		float heuristic(sf::Vector2u const & pos) const;
		void insert(PathNode const & node);
		
	public:
		/// @brief Initial pathfinder for a single scene
		///
		/// @param scene Reference to the underlying scene
		Pathfinder(Scene const& scene);
		
		/// @brief Calculate a path
		///
		/// @param entity_id Actor's ID
		/// @param origin Source position
		/// @param target Target position
		/// @param max_length Maximum length for desired path
		/// @param ignore Vector of Entities to ignore
		Path operator()(Entity entity_id, sf::Vector2u const& origin,
			sf::Vector2u const& target, std::size_t max_length,
			std::vector<Entity> const& ignore={});
};

/*
enum class NodeStatus { Unknown, Open, Closed };

struct PathNode {
	sf::Vector2u pos;
	PathNode *previous, *next;
	float f, g;
	NodeStatus status;

	PathNode(sf::Vector2u pos = {0u, 0u});
};

class PathDict {
  private:
	sf::Vector2u const grid_size;
	std::vector<PathNode> nodes;

  public:
	PathDict(sf::Vector2u const& grid_size);

	void clear();
	PathNode& at(sf::Vector2u const& pos);
};

// ---------------------------------------------------------------------------

struct CoordHash {
	sf::Vector2u const grid_size;

	CoordHash(sf::Vector2u const& grid_size);

	std::size_t range() const;
	std::size_t operator()(sf::Vector2u const& pos) const;
};

// ---------------------------------------------------------------------------

template <typename Scene, typename Entity>
class Pathfinder {
  private:
	Scene const& scene;

	PathDict dict;
	PriorityQueue<sf::Vector2u, float, CoordHash> queue;

	sf::Vector2u origin, target;
	PathNode* nearest;
	float alt;  // distance between nearest and target
	std::size_t max_length;
	bool strict;
	std::vector<sf::Vector2u> path, neighbors;
	Entity entity_id;

  public:
	Pathfinder(Scene const& scene);

	void initialize(Entity entity_id, sf::Vector2u const& origin,
		sf::Vector2u const& target, std::size_t max_length, bool strict = true);

	/// Calculate next step of pathfinding
	/// @pre Pathfinding was initialized for a specific entity
	///	@return true if path found (or everything was checked)
	///
	bool calculate();

	std::vector<sf::Vector2u> getPath() const;
};
*/

}  // ::utils

// include implementation details
#include <utils/pathfinder.inl>

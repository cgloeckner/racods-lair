#include <utils/assert.hpp>

#include <core/collision.hpp>
#include <core/teleport.hpp>
#include <game/navigator.hpp>

namespace game {

namespace navigator_impl {

float distance(sf::Vector2u const& u, sf::Vector2u const& v) {
	auto dx = utils::distance(u.x, v.x);
	auto dy = utils::distance(u.y, v.y);
	auto max = std::max(dx, dy);
	auto min = std::min(dx, dy);
	// max-min		: straight distance
	// min * 1.414f	: diagonale distance
	return (max - min) + min * 1.414f;
}

}  // ::navigator_impl

// ---------------------------------------------------------------------------

NavigationNode::NavigationNode(sf::Vector2u const& offset)
	: offset{offset}, paths{} {}

// ---------------------------------------------------------------------------

DungeonGraph::DungeonGraph(sf::Vector2u const& size)
	: size{size}, nodes{size.x * size.y} {}

DungeonGraph::DungeonGraph(DungeonGraph&& other)
	: size{std::move(other.size)}, nodes{std::move(other.nodes)} {}

void DungeonGraph::addNode(sf::Vector2u const& cell_pos) {
	ASSERT(cell_pos.x < size.x);
	ASSERT(cell_pos.y < size.y);

	auto& uptr = nodes[cell_pos.x + cell_pos.y * size.x];
	ASSERT(uptr == nullptr);
	uptr = std::make_unique<NavigationNode>(cell_pos);
}

void DungeonGraph::addPath(sf::Vector2u const& src, sf::Vector2u const& dst) {
	ASSERT(src.x < size.x);
	ASSERT(src.y < size.y);
	ASSERT(dst.x < size.x);
	ASSERT(dst.y < size.y);

	auto& a = nodes[src.x + src.y * size.x];
	auto& b = nodes[dst.x + dst.y * size.x];
	ASSERT(a != nullptr);
	ASSERT(b != nullptr);
	if (utils::contains(a->paths, b.get())) {
		ASSERT(utils::contains(b->paths, a.get()));
		return;
	}
	a->paths.push_back(b.get());
	b->paths.push_back(a.get());
}

NavigationNode* DungeonGraph::getNode(sf::Vector2u const& cell_pos) {
	ASSERT(cell_pos.x < size.x);
	ASSERT(cell_pos.y < size.y);

	auto& uptr = nodes[cell_pos.x + cell_pos.y * size.x];
	if (uptr == nullptr) {
		return nullptr;
	}
	return uptr.get();
}

NavigationNode const* DungeonGraph::getNode(
	sf::Vector2u const& cell_pos) const {
	ASSERT(cell_pos.x < size.x);
	ASSERT(cell_pos.y < size.y);

	auto& uptr = nodes[cell_pos.x + cell_pos.y * size.x];
	if (uptr == nullptr) {
		return nullptr;
	}
	return uptr.get();
}

float DungeonGraph::getDistance(
	sf::Vector2u const& u, sf::Vector2u const& v) const {
	return navigator_impl::distance(u, v);
}

sf::Vector2u DungeonGraph::getSize() const { return size; }

std::vector<sf::Vector2u> DungeonGraph::getNeighbors(
	core::ObjectID actor, sf::Vector2u const& pos,
	std::vector<core::ObjectID> const& ignore) const {
	std::vector<sf::Vector2u> neighbors;
	auto const& node = nodes[pos.x + pos.y * size.x];
	if (node != nullptr) {
		for (auto ptr : node->paths) {
			neighbors.push_back(ptr->offset);
		}
	}
	return neighbors;
}

// ---------------------------------------------------------------------------

NavigationScene::NavigationScene(
	core::CollisionManager const& collision, core::Dungeon const& dungeon)
	: collision{collision}, dungeon{dungeon} {}

float NavigationScene::getDistance(
	sf::Vector2u const& u, sf::Vector2u const& v) const {
	return navigator_impl::distance(u, v);
}

sf::Vector2u NavigationScene::getSize() const { return dungeon.getSize(); }

std::vector<sf::Vector2u> NavigationScene::getNeighbors(
	core::ObjectID actor, sf::Vector2u const& pos,
	std::vector<core::ObjectID> const& ignore) const {
	std::vector<sf::Vector2u> straight, neighbors;
	if (!collision.has(actor)) {
		// actor seems to be dead
		return neighbors;
	}
	auto const& coll_data = collision.query(actor);

	sf::Vector2i delta;
	for (delta.y = -1; delta.y <= 1; ++delta.y) {
		for (delta.x = -1; delta.x <= 1; ++delta.x) {
			if (delta.x == 0 && delta.y == 0) {
				// ignore: invalid direction
				continue;
			}
			auto next = sf::Vector2u{sf::Vector2i{pos} + delta};
			if (!dungeon.has(next)) {
				// ignore: invalid pos
				continue;
			}
			auto const& cell = dungeon.getCell(next);
			if (core::checkTileCollision(cell)) {
				// ignore: tile collision
				continue;
			}
			if (cell.trigger != nullptr && dynamic_cast<core::TeleportTrigger*>(cell.trigger.get()) != nullptr) {
				// ignore: teleport triggers
				continue;
			}
			auto colliders = core::checkObjectCollision(collision, cell, coll_data);
			bool hit{false};
			for (auto id: colliders) {
				if (!utils::contains(ignore, id)) {
					// ignore: object collision
					hit = true;
					continue;
				}
				if (hit) {
					break;
				}
			}
			if (hit) {
				continue;
			}
			
			// add position (categorized by kind of direction)
			if (delta.x * delta.y == 0) {
				// straight moves are stored indirectly
				straight.push_back(next);
			} else {
				// diagonal moves are stored directly
				neighbors.push_back(next);
			}
		}
	}
	
	// add straights to neighbors (so they get a lower priority)
	utils::append(neighbors, straight);

	return neighbors;
}

bool NavigationScene::canAccess(core::ObjectID actor,
	sf::Vector2u const& pos) const {
	if (!dungeon.has(pos)) {
		// ignore: invalid pos
		return false;
	}
	auto const& cell = dungeon.getCell(pos);
	if (core::checkTileCollision(cell)) {
		// ignore: tile collision
		return false;
	}
	if (!collision.has(actor)) {
		// actor seems to be dead
		return false;
	}
	auto const& coll_data = collision.query(actor);
	if (!core::checkObjectCollision(collision, cell, coll_data).empty()) {
		// ignore: object collision
		return false;
	}
	return true;
}

// ---------------------------------------------------------------------------

Navigator::Navigator(DungeonGraph&& graph, NavigationScene&& scene)
	: graph{std::move(graph)}
	, scene{std::move(scene)}
	, broadphase{this->graph}
	, narrowphase{this->scene} {
}

// ---------------------------------------------------------------------------

NavigationSystem::NavigationSystem()
	: navis{} {
}

Navigator& NavigationSystem::create(utils::SceneID id,
	core::CollisionManager const& collision, core::Dungeon const& dungeon,
	DungeonBuilder const& builder) {
	ASSERT(id > 0u);
	ASSERT(navis.size() == id - 1u);
	// create graph
	DungeonGraph graph{builder.grid_size};
	for (auto const& path : builder.paths) {
		if (graph.getNode(path.origin) == nullptr) {
			graph.addNode(path.origin);
		}
		if (graph.getNode(path.target) == nullptr) {
			graph.addNode(path.target);
		}
		graph.addPath(path.origin, path.target);
	}
	// create scene
	NavigationScene scene{collision, dungeon};
	// create navigation
	navis.push_back(nullptr);
	auto& tmp = navis.back();
	tmp = std::make_unique<Navigator>(std::move(graph), std::move(scene));
	return *tmp;
}

Navigator& NavigationSystem::operator[](utils::SceneID id) {
	ASSERT(id > 0u);
	return *navis.at(id - 1u);
}

void NavigationSystem::clear() {
	navis.clear();
}

}  // ::game

#include <algorithm>
#include <iostream>

namespace utils {

template <typename Scene, typename Entity>
Pathfinder<Scene, Entity>::Pathfinder(Scene const& scene)
	: scene{scene}
	, compare{}
	, openlist{}
	, closedlist{}
	, entity_id{}
	, origin{}
	, target{}
	, max_length{} {
}

template <typename Scene, typename Entity>
float Pathfinder<Scene, Entity>::heuristic(sf::Vector2u const & pos) const {
	return scene.getDistance(pos, target);
}

template <typename Scene, typename Entity>
void Pathfinder<Scene, Entity>::insert(PathNode const & node) {
	openlist.push_back(node);
	std::push_heap(openlist.begin(), openlist.end(), compare);
}

template <typename Scene, typename Entity>
Path Pathfinder<Scene, Entity>::operator()(Entity entity_id,
	sf::Vector2u const& origin, sf::Vector2u const& target,
	std::size_t max_length, std::vector<Entity> const& ignore) {
	Path p;
	
	// apply request
	this->entity_id = entity_id;
	this->origin = origin;
	this->target = target;
	this->max_length = max_length;
	
	// enqueue origin with f = h
	PathNode start{origin};
	start.f = heuristic(origin);
	insert(start);
	
	// search
	PathNode const* closest{nullptr};
	
	while (!openlist.empty()) {
		// extract min
		std::pop_heap(openlist.begin(), openlist.end(), compare);
		auto node = openlist.back();
		openlist.pop_back();
		
		if (node.pos == target) {
			// reconstruct path to target
			closest = &node;
			break;
		}
		
		auto it = closedlist.insert(node);
		if (!it.second) {
			// position already on closed list - skip further stuff
			continue;
		}
		
		if (closest == nullptr || heuristic(it.first->pos) < heuristic(closest->pos)) {
			// pointer to the closest yet discovered position
			closest = &(*it.first);
		}
		
		// expand node
		auto neighbors = scene.getNeighbors(entity_id, node.pos, ignore);
		for (auto const& neighbor_pos: neighbors) {
			if (closedlist.find(neighbor_pos) != closedlist.end()) {
				continue;
			}
			
			// evaluate found node
			PathNode n{neighbor_pos};
			n.g = node.g + scene.getDistance(node.pos, neighbor_pos);
			n.f = n.g + heuristic(neighbor_pos);
			n.previous = &(*it.first);
			
			// respect maximum path length
			if (max_length > 0u && n.g > max_length) {
				closedlist.insert(n);
				continue;
			}
			
			// insert node again (despite it is at openlist or not)
			// note: insert is performed here instead decrease key,
			// if the node is processed twice, its already closed yet
			insert(n);
		}
	}
	
	// reconstruct path
	while (closest != nullptr) {
		p.push_back(closest->pos);
		closest = closest->previous;
	}
	
	openlist.clear();
	closedlist.clear();
	
	return p;
}

}  // ::utils

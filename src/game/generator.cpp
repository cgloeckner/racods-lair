#include <utils/algorithm.hpp>

#include <game/generator.hpp>

namespace game {

DungeonData::DungeonData(sf::Vector2u const& grid_size, sf::Vector2u const& layout_size)
	: builder{grid_size}
	, graph{layout_size} {
}

// ---------------------------------------------------------------------------

DungeonGenerator::DungeonGenerator(core::LogContext& log)
	: log{log}
	, data{}
	, settings{}
	, rooms{} {
}

void DungeonGenerator::layoutifySize(sf::Vector2u& grid_size) {
	sf::Vector2u layout_size;
	layout_size.x = static_cast<unsigned int>(
		std::ceil(1.f * grid_size.x / settings.cell_size));
	layout_size.y = static_cast<unsigned int>(
		std::ceil(1.f * grid_size.y / settings.cell_size));
	grid_size.x = layout_size.x * settings.cell_size;
	grid_size.y = layout_size.y * settings.cell_size;
}

DungeonData& DungeonGenerator::generate(utils::SceneID id, sf::Vector2u grid_size) {
	// note: assuming synchronicity with core::DungeonSystem
	ASSERT(id > 0u);
	ASSERT(data.size() + 1u == id);
	
	settings.verify();
	ASSERT(!rooms.empty());
	for (auto const & ptr: rooms) {
		ASSERT(ptr != nullptr);
		ASSERT(ptr->isValid(log.debug, settings.cell_size));
	}

	sf::Vector2u layout_size;
	layout_size.x = static_cast<unsigned int>(
		std::ceil(1.f * grid_size.x / settings.cell_size));
	layout_size.y = static_cast<unsigned int>(
		std::ceil(1.f * grid_size.y / settings.cell_size));

	auto uptr = std::make_unique<DungeonData>(grid_size, layout_size);

	// determine number of nodes (rooms + deadends)
	auto const num_cells = layout_size.x * layout_size.y;
	auto const node_density = settings.room_density + settings.deadend_density;
	auto const num_nodes = static_cast<std::size_t>(std::ceil(node_density * num_cells));

	// determine distribution of rooms and deadends
	auto num_rooms = static_cast<std::size_t>(std::ceil(settings.room_density * num_cells));
	auto num_deadends = static_cast<std::size_t>(
		std::ceil(settings.deadend_density * num_cells));

	std::vector<sf::Vector2i> directions;
	directions.emplace_back(1, 0);
	directions.emplace_back(-1, 0);
	directions.emplace_back(0, 1);
	directions.emplace_back(0, -1);

	std::vector<sf::Vector2u> openlist;
	std::vector<bool> nodes;
	std::vector<std::pair<sf::Vector2u, sf::Vector2u>> paths, redundant;
	nodes.resize(num_cells, false);

	auto set = [&](sf::Vector2u const& pos) {
		openlist.push_back(pos);
		nodes[pos.x + pos.y * layout_size.x] = true;
	};

	// start at random cell
	set({thor::random(0u, layout_size.x - 1u),
		thor::random(0u, layout_size.y - 1u)});
	auto i = 1u;
	while (i < num_nodes) {
		// pick random node
		auto const j = thor::random(0u, openlist.size() - 1u);
		auto const origin = openlist[j];
		// pick a random direction that leads to open space
		utils::shuffle(directions);
		bool found{false};
		for (auto const& dir : directions) {
			auto pos = sf::Vector2u{sf::Vector2i{origin} + dir};
			if (pos.x >= layout_size.x || pos.y >= layout_size.y) {
				// invalid pos
				continue;
			}
			if (nodes[pos.x + pos.y * layout_size.x]) {
				// no open space
				redundant.emplace_back(origin, pos);
				continue;
			}
			found = true;
			set(pos);
			paths.emplace_back(origin, pos);
			break;
		}
		if (found) {
			++i;
		} else {
			// try another position
			utils::pop(openlist, origin);
			if (openlist.empty()) {
				log.error << "[Game/Generator] " << "The force has not been with the RNG oO\n";
				break;
			}
		}
	}

	// build actual rooms
	sf::Vector2u pos;
	i = 0u;
	auto origin = settings.cell_size / 2u;
	for (pos.y = 0u; pos.y < layout_size.y; ++pos.y) {
		for (pos.x = 0u; pos.x < layout_size.x; ++pos.x) {
			if (!nodes[pos.x + pos.y * layout_size.x]) {
				continue;
			}

			uptr->graph.addNode(pos);

			// determine whether node is a room or a deadend
			auto const n = num_rooms + num_deadends;
			if (n == 0u) {
				// all rooms and deadends have been placed
				break;
			}
			auto const j = thor::random(0u, n);
			if (j <= num_rooms) {
				// create global offset
				unsigned int left = pos.x * settings.cell_size;
				unsigned int top = pos.y * settings.cell_size;
				// pick random room
				auto index = thor::random(0u, rooms.size()-1u);
				auto ptr = rooms[index];
				//log.debug << "[Game/Generator] " << "Using room " << (index+1) << " / " << rooms.size() << "\n";
				// add room
				uptr->builder.rooms.emplace_back(left, top, *ptr);
				--num_rooms;

			} else {
				//log.debug << "[Game/Generator] " << "creating deadend\n";
				--num_deadends;
			}
		}
	}

	// pick some redundant paths
	if (!redundant.empty()) {
		auto num_redundant_paths = static_cast<std::size_t>(
			std::ceil(settings.redundant_paths_ratio * num_cells));
		utils::shuffle(redundant);
		for (auto i = 0u; i < num_redundant_paths; ++i) {
			auto pair = redundant.back();
			redundant.pop_back();
			paths.push_back(pair);
			if (redundant.empty()) {
				break;
			}
		}
	}

	// build actual paths
	for (auto const& pair : paths) {
		uptr->graph.addPath(pair.first, pair.second);
		// note: positions are made global to the dungeon
		auto src = pair.first;
		src.x *= settings.cell_size;
		src.y *= settings.cell_size;
		src.x += origin;
		src.y += origin;
		auto dst = pair.second;
		dst.x *= settings.cell_size;
		dst.y *= settings.cell_size;
		dst.x += origin;
		dst.y += origin;
		uptr->builder.paths.emplace_back(src, dst);
	}
	
	data.push_back(std::move(uptr));
	return *data.back();
}

DungeonData& DungeonGenerator::operator[](utils::SceneID id) {
	ASSERT(id > 0u);
	ASSERT(id <= data.size());
	auto& ptr = data[id - 1u];
	ASSERT(ptr != nullptr);
	return *ptr;
}

DungeonData const & DungeonGenerator::operator[](utils::SceneID id) const {
	ASSERT(id > 0u);
	ASSERT(id <= data.size());
	auto const & ptr = data[id - 1u];
	ASSERT(ptr != nullptr);
	return *ptr;
}

void DungeonGenerator::clear() {
	data.clear();
}

} // ::game

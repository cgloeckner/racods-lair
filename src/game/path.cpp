#include <SFML/System/Clock.hpp>

#include <utils/assert.hpp>
#include <utils/algorithm.hpp>
#include <core/teleport.hpp>
#include <game/path.hpp>

namespace game {

namespace path_impl {

std::size_t const MAX_PATH_LENGTH = 30u;
Request::Request()
	: actor{0u}
	, scene{0u}
	, source{}
	, target{}
	, path{} {}

}  // ::path_impl

// ---------------------------------------------------------------------------

PathSystem::PathSystem(core::LogContext& log)
	: log{log}
	, scenes{}
	, requests{} {}

void PathSystem::addScene(utils::SceneID id, Navigator& navigator) {
	scenes.resize(id + 1u);
	// register scene with navigation
	scenes[id] = &navigator;
}

std::future<Path> PathSystem::schedule(core::ObjectID actor,
	utils::SceneID scene, sf::Vector2u const& source,
	sf::Vector2u const& target) {
	ASSERT(scenes.size() > scene);
	ASSERT(scenes[scene] != nullptr);

	path_impl::Request* request{nullptr};
	auto i = requests.begin();
	++i; // ignore front
	for (; i != requests.end(); ++i) {
		if (i->actor == actor) {
			request = &*i;
			break;
		}
	}

	if (request == nullptr) {
		// create new request
		requests.emplace_back();
		request = &requests.back();
	}

	// apply data to request
	request->path = std::promise<Path>{};
	request->actor = actor;
	request->scene = scene;
	request->source = source;
	request->target = target;

	return request->path.get_future();
}

std::size_t PathSystem::calculate(sf::Time const& max_elapse) {
	sf::Clock clock;
	sf::Time min{max_elapse * 10.f}, max{sf::Time::Zero}, avg{sf::Time::Zero};
	
	for (auto& current: requests) {
		// determine maximum path length
		auto dist = static_cast<unsigned int>(std::ceil(navigator_impl::distance(current.source, current.target)));
		auto max_length = std::max(20u, dist * 3u);
		
		// trigger pathfinding
		auto& navi = *scenes[current.scene];
		auto path = navi.narrowphase(current.actor, current.source, current.target, max_length);
		current.path.set_value(std::move(path));
		
		auto delta = clock.restart();
		min = std::min(min, delta);
		max = std::max(max, delta);
		avg += delta;
	}
	
	auto n = requests.size();
	requests.clear();
	
	if (n > 0u && max > sf::milliseconds(5u)) {
		log.debug << "[Game/Path] " << n << " paths calculated within "
			<< avg << " (min: " << min << ", max: " << max << ", avg: "
			<< (avg / (1.f * n)) << ")\n";
	}
	
	return n;
}

}  // ::rage

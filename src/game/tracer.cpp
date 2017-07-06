#include <utils/assert.hpp>
#include <core/algorithm.hpp>
#include <game/tracer.hpp>
#include <game/navigator.hpp>  // distance!

namespace game {

PathTracer::PathTracer(core::LogContext& log, PathSystem& pathfinder,
	core::MovementManager const& movement_manager,
	core::InputSender& input_sender, core::ObjectID actor)
	: state{PathTracer::Idle}
	, log{log}
	, pathfinder{pathfinder}
	, movement_manager{movement_manager}
	, input_sender{input_sender}
	, actor{actor}
	, request{}
	, path{}
	, current{}
	, target{} {}

bool PathTracer::requestIsReady() const {
	auto status = request.wait_for(std::chrono::milliseconds(0));
	return status == std::future_status::ready;
}

void PathTracer::reset() {
	state = PathTracer::Idle;
	request = std::future<Path>{};
	path.clear();
	current = sf::Vector2u{};
	// target = sf::Vector2u{};
	(void)log;
}

void PathTracer::pathfind(sf::Vector2u const& target) {
	this->target = target;
	// force path renew
	auto const& data = movement_manager.query(actor);
	current = data.target;
	request = std::future<Path>{};
	path.clear();
	state = PathTracer::Trigger;
}

void PathTracer::handle(core::MoveEvent const& event) {
	switch (event.type) {
		case core::MoveEvent::Left: {
			// stop movement
			core::InputEvent event;
			event.actor = actor;
			event.move = {};
			input_sender.send(event);
		} break;

		case core::MoveEvent::Reached:
			current = event.target;
			if (state != PathTracer::Trace) {
				return;
			}
			if (path.empty()) {
				log.debug << "[Game/Tracer] " << "Panic: #" << actor << " has no path\n";
				return;
			}
			if (path.back() == current) {
				path.pop_back();
			}
			if (!path.empty()) {
				// head to next waypiont
				auto next = path.back();
				auto v = sf::Vector2i{next} - sf::Vector2i{current};
				core::fixDirection(v);
				core::InputEvent event;
				event.actor = actor;
				event.move = v;
				event.look = v;
				input_sender.send(event);
			} else {
				state = PathTracer::Idle;
			}
			break;
	}
}

void PathTracer::handle(core::CollisionEvent const& event) {
	// force abort
	request = std::future<Path>{};
	path.clear();
	state = PathTracer::Idle;
}

boost::optional<PathFailedEvent> PathTracer::update() {
	switch (state) {
		case PathTracer::Idle:
			break;

		case PathTracer::Trigger: {
			if (target.x == 0 and target.y == 0) {
				log.debug << "[Game/Tracer] " << "Warning: invalid target\n";
			}
			auto const& data = movement_manager.query(actor);
			current = sf::Vector2u{data.pos};
			request = pathfinder.schedule(actor, data.scene, current, target);
			state = PathTracer::Wait;
		} break;

		case PathTracer::Wait:
			ASSERT(request.valid());
			if (requestIsReady()) {
				path = request.get();
				// check whether path is ok
				if (path.size() == 1u &&
					navigator_impl::distance(current, target) >= 2.f) {
					// pathfinding failed!
					state = PathTracer::Idle;
					PathFailedEvent event;
					event.actor = actor;
					event.pos = target;
					return event;

				} else {
					state = PathTracer::Trace;
					core::MoveEvent event;
					event.source = current;
					event.target = current;
					event.type = core::MoveEvent::Reached;
					handle(event);
				}
			}
			break;

		case PathTracer::Trace:
			// tracing is done on entering a tile
			break;
	}

	return boost::none;
}

bool PathTracer::isRunning() const { return state != PathTracer::Idle; }

std::vector<sf::Vector2u> const& PathTracer::getPath() const { return path; }

}  // ::rage

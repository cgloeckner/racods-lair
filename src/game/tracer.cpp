#include <utils/algorithm.hpp>
#include <utils/assert.hpp>
#include <utils/math2d.hpp>
#include <core/algorithm.hpp>
#include <game/tracer.hpp>

namespace game {

namespace tracer_impl {

float const WAYPOINT_REACHED_THRESHOLD = 0.01f;

Context::Context(core::LogContext& log, core::InputSender& input_sender, core::MovementManager const & movement, PathSystem& pathfinder)
	: log{log}
	, input_sender{input_sender}
	, movement{movement}
	, pathfinder{pathfinder} {
}

void onCollision(Context& context, TracerData& data) {
	if (!data.is_enabled) {
		return;
	}
	
	auto const & move_data = context.movement.query(data.id);
	auto target  = sf::Vector2f{data.path.front()};
	
	// Trigger pathfinding from current position to the original target
	tracer(context.log, context.pathfinder, context.movement.query(data.id), data, target);
}

void onTeleport(TracerData& data) {
	data.request = std::future<Path>{};
	data.path.clear();
}

void onDeath(TracerData& data) {
	data.request = std::future<Path>{};
	data.path.clear();
	data.is_enabled = false;
}

void onSpawn(TracerData& data) {
	data.request = std::future<Path>{};
	data.path.clear();
	data.is_enabled = true;
}

void onUpdate(Context& context, TracerData& data) {
	// check if path tracing is possible
	if (!data.is_enabled) {
		return;
	}
	if (data.path.empty() && !utils::isReady(data.request)) {
		return;
	}
	
	bool trigger{false};
	if (data.path.empty()) {
		data.path = std::move(data.request.get());
		trigger = !data.path.empty();
	}
	
	// check whether object moved close enough
	auto const & move_data = context.movement.query(data.id);
	auto next       = sf::Vector2f{data.path.back()} + utils::HalfTilePos;
	auto dist       = utils::distance(move_data.pos, next);
	auto last_dist  = utils::distance(move_data.last_pos, next);
	bool wp_reached = dist < WAYPOINT_REACHED_THRESHOLD;
	bool wp_missed  = last_dist < dist; // last position was closed then current
	
	if (wp_reached || wp_missed) {
		// waypoint was reached
		data.path.pop_back();
		trigger = true;
		context.log.debug << "[Game/Tracer] " << "Waypoint " << next << " reached by #"
			<< data.id << "\n";
	}
	
	// trigger trace of next waypoint if suitable
	if (trigger && !data.path.empty()) {
		next = sf::Vector2f{data.path.back()} + utils::HalfTilePos;
		if (next == move_data.pos) {
			// ignore this wp
			data.path.pop_back();
			return;
		}
		
		core::InputEvent event;
		event.actor = data.id;
		event.move = thor::unitVector(next - move_data.pos);
		event.look = event.move;
		context.input_sender.send(event);
		
		context.log.debug << "[Game/Tracer] " << "Going to " << next << " via " << event.move << "\n";
	}
}

} // ::tracer_impl


void tracer(core::LogContext& log, PathSystem& pathfinder, core::MovementData const & move, TracerData& tracer, sf::Vector2f const & target) {
	if (!tracer.is_enabled) {
		return;
	}
	
	tracer.request = pathfinder.schedule(tracer.id, move.scene, sf::Vector2u{move.pos}, sf::Vector2u{target});
	tracer.path.clear();
	
	log.debug << "[Game/Tracer] " << "Pathfinding #" << tracer.id << " to " << target << "\n";
}

// ---------------------------------------------------------------------------------------

TracerSystem::TracerSystem(core::LogContext& log, std::size_t max_objects, core::MovementManager const & movement,
		PathSystem& pathfinder)
	: utils::EventListener<core::CollisionEvent, core::TeleportEvent, rpg::DeathEvent, rpg::SpawnEvent>{}
	, utils::EventSender<core::InputEvent>{}
	, TracerManager{max_objects}
	, context{log, *this, movement, pathfinder} {
}

void TracerSystem::handle(core::CollisionEvent const& event) {
	if (!event.interrupt) {
		return;
	}
	
	if (!has(event.actor)) {
		return;
	}
	
	auto& data = query(event.actor);
	tracer_impl::onCollision(context, data);
}

void TracerSystem::handle(core::TeleportEvent const& event) {
	if (!has(event.actor)) {
		return;
	}
	
	auto& data = query(event.actor);
	tracer_impl::onTeleport(data);
}

void TracerSystem::handle(rpg::DeathEvent const& event) {
	if (!has(event.actor)) {
		return;
	}
	
	auto& data = query(event.actor);
	tracer_impl::onDeath(data);
}

void TracerSystem::handle(rpg::SpawnEvent const& event) {
	if (!has(event.actor)) {
		return;
	}
	
	auto& data = query(event.actor);
	tracer_impl::onSpawn(data);
}

void TracerSystem::update(sf::Time const& elapsed) {
	dispatch<core::CollisionEvent>(*this);
	dispatch<core::TeleportEvent>(*this);
	dispatch<rpg::DeathEvent>(*this);
	dispatch<rpg::SpawnEvent>(*this);

	for (auto& data: *this) {
		tracer_impl::onUpdate(context, data);
	}
	
	propagate<core::InputEvent>();
}

} // ::game

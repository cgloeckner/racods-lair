#include <utils/assert.hpp>
#include <utils/math2d.hpp>

#include <core/algorithm.hpp>
#include <core/movement.hpp>

namespace core {

namespace movement_impl {

float const MOVEMENT_VELOCITY = 0.0001f;
float const MIN_SPEEDFACTOR = 0.25f;
float const MAX_SPEEDFACTOR = 1.75f;
float const DELTA_SPEEDFACTOR = 0.05f;
float const SIDEWARD_SPEEDFACTOR = 0.9f;
float const BACKWARD_SPEEDFACTOR = 0.75f;

Context::Context(LogContext& log, MoveSender& move_sender,
	MovementManager& movement_manager, DungeonSystem& dungeon_system)
	: log{log}
	, move_sender{move_sender}
	, movement_manager{movement_manager}
	, dungeon_system{dungeon_system} {}

// ---------------------------------------------------------------------------

void setMovement(Context& context, MovementData& actor, sf::Vector2f const & move, sf::Vector2f const & look) {
	auto was_moving = (actor.move != sf::Vector2f{});
	auto will_move  = (move != sf::Vector2f{});
	
	// apply normalized vector
	actor.move = utils::normalize(move);
	actor.look = utils::normalize(look);
	actor.has_changed = true;
	
	// propagate event if behavior changed
	if (was_moving != will_move) {
		MoveEvent event;
		event.actor = actor.id;
		event.type  = will_move ? MoveEvent::Start : MoveEvent::Stop;
		context.move_sender.send(event);
	}
}

void onCollision(Context& context, MovementData& actor, CollisionEvent const & event) {
	if (event.interrupt) {
		actor.pos         = actor.last_pos;
		actor.has_changed = true;
		movement_impl::setMovement(context, actor, {}, actor.look);
	}
}

MoveStyle getMoveStyle(MovementData const & actor) {
	auto angle = std::abs(thor::signedAngle(actor.move, actor.look));
	if (angle < 45.f) {
		return MoveStyle::Forward;
	} else if (angle < 135.f) {
		return MoveStyle::Sideward;
	} else {
		return MoveStyle::Backward;
	}
}

float calcSpeedFactor(MovementData const& actor) {
	// calculate speed_factor
	float speed_factor = 1.f + movement_impl::DELTA_SPEEDFACTOR * actor.num_speed_boni;
	
	// consider movement style
	auto style = getMoveStyle(actor);
	switch (style) {
		case MoveStyle::Backward:
			speed_factor *= movement_impl::BACKWARD_SPEEDFACTOR;
			break;
			
		case MoveStyle::Sideward:
			speed_factor *= movement_impl::SIDEWARD_SPEEDFACTOR;
			break;
			
		case MoveStyle::Forward:
			// no change
			break;
	}
	
	if (speed_factor < movement_impl::MIN_SPEEDFACTOR) {
		speed_factor = movement_impl::MIN_SPEEDFACTOR;
	} else if (speed_factor > MAX_SPEEDFACTOR) {
		speed_factor = MAX_SPEEDFACTOR;
	}
	ASSERT(speed_factor >= movement_impl::MIN_SPEEDFACTOR);
	ASSERT(speed_factor <= MAX_SPEEDFACTOR);
	
	return speed_factor;
}

void interpolate(Context& context, MovementData& data, sf::Time const& elapsed) {
	if (data.scene == 0u) {
		// object already vanished
		return;
	}
	ASSERT(data.max_speed >= 0.f);
	ASSERT(data.max_speed <= MAX_SPEED);
	
	// interpolate movement
	float delta{1.f};
	if (data.move != sf::Vector2f{}) {
		delta = data.max_speed * calcSpeedFactor(data) * elapsed.asSeconds();
	}
	
	// apply changes
	data.last_pos = data.pos;
	data.pos = data.pos + delta * data.move;
	data.has_changed = true;
}

void updateRange(Context& context, MovementManager::iterator begin,
	MovementManager::iterator end, sf::Time const& elapsed) {
	for (auto i = begin; i != end; ++i) {
		auto& data = *i;
		if (data.move != sf::Vector2f{}) {
			movement_impl::interpolate(context, data, elapsed);
		}
	}
}


}  // ::movement_impl

// ---------------------------------------------------------------------------

MovementSystem::MovementSystem(LogContext& log, std::size_t max_objects,
	DungeonSystem& dungeon)
	// Event API
	: utils::EventListener<InputEvent, CollisionEvent>{}
	, utils::EventSender<MoveEvent>{}  // Component API
	, MovementManager{max_objects}
	, context{log, *this, *this, dungeon} {}

void MovementSystem::handle(InputEvent const& event) {
	if (!has(event.actor)) {
		// object has no movement component
		// note: object might have been deleted
		return;
	}
	auto& actor = query(event.actor);
	movement_impl::setMovement(context, actor, event.move, event.look);
}

void MovementSystem::handle(CollisionEvent const& event) {
	if (!has(event.actor)) {
		// object has no movement component
		// note: object might have been deleted
		return;
	}
	auto& actor = query(event.actor);
	movement_impl::onCollision(context, actor, event);
}

void MovementSystem::update(sf::Time const& elapsed) {
	dispatch<InputEvent>(*this);
	dispatch<CollisionEvent>(*this);

	updateRange(context, begin(), end(), elapsed);

	propagate<MoveEvent>();
}

}  // ::core

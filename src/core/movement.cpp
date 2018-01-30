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

// to fix floating point inaccuracy when reaching tile
float const MOVEMENT_ACCURACY = 0.0001f;

// to determine whether an object is centered on a cell or not
float const CELL_CENTER_DIVERGENCE = 0.00001f;

Context::Context(LogContext& log, MoveSender& move_sender,
	MovementManager& movement_manager, DungeonSystem& dungeon_system)
	: log{log}
	, move_sender{move_sender}
	, movement_manager{movement_manager}
	, dungeon_system{dungeon_system} {}

// ---------------------------------------------------------------------------

void updateRange(Context& context, MovementManager::iterator begin,
	MovementManager::iterator end, sf::Time const& elapsed) {
	for (auto i = begin; i != end; ++i) {
		auto& data = *i;
		if (data.move != sf::Vector2i{} || data.next_move != sf::Vector2i{}) {
			movement_impl::interpolate(context, data, elapsed);
		}
	}
}

void start(Context& context, MovementData& data, InputEvent const& event) {
	// apply direction for next move
	data.next_move = event.move;
	data.is_moving = data.move != sf::Vector2i{} || data.next_move != sf::Vector2i{};

	if (event.look.x != 0 || event.look.y != 0) {
		data.look = event.look;
		data.has_changed = true;
	}
}

void moveToTarget(Context& context, MovementData& data) {
	ASSERT(data.scene > 0u);
	auto& dungeon = context.dungeon_system[data.scene];
	auto source = data.target;
	auto target = sf::Vector2u{sf::Vector2i{source} + data.next_move};

	auto tmp = sf::Vector2u{data.pos};
	if (tmp != source) {
		context.log.debug << "[Core/Movement] " << "Object #" << data.id << " is located at "
						  << data.pos << " but assigned to " << source
						  << " instead of " << tmp << "\n";
		ASSERT(utils::contains(dungeon.getCell(source).entities, data.id));
	}

	ASSERT(dungeon.has(source));
	if (!dungeon.has(target)) {
		// target not found, stop movement!
		CollisionEvent event;
		event.pos = source;
		event.reset_to = source;
		event.interrupt = true;
		stop(context, data, event);
		return;
	}

	if (data.next_move == sf::Vector2i{}) {
		// nothing more to do
		return;
	}

	// prepare movement
	data.move = data.next_move;
	//data.is_moving = data.move != sf::Vector2i{};
	data.target = target;

	// propagate leaving tile
	MoveEvent event;
	event.actor = data.id;
	event.source = source;
	event.target = target;
	event.type = MoveEvent::Left;
	context.move_sender.send(event);
}

void stop(Context& context, MovementData& data, CollisionEvent const& event) {
	if (!event.interrupt) {
		// nothing to do
		return;
	}
	
	if (data.scene == 0u) {
		// object vanished yet
		return;
	}
	
	/// collision map handling is part of collision system!!!
	/*
	auto const& dungeon = context.dungeon_system[data.scene];
	if (event.pos != event.reset_to) {
		// assert object is not located at the previous target position anymore
		ASSERT(!utils::contains(dungeon.getCell(event.pos).entities, data.id));
	}
	// assert object is already located at the reset-position
	ASSERT(utils::contains(dungeon.getCell(event.reset_to).entities, data.id));
	*/
	
	// reset position
	//data.pos = sf::Vector2f{event.reset_to};
	//data.target = event.reset_to;
	
	data.pos    = data.last_pos;
	data.is_moving = false;
	data.target = sf::Vector2u{data.pos};
	data.move   = sf::Vector2i{}; /// @TODO Vector2f after overhaul
	data.next_move = data.move;
	data.has_changed = true;
	
	// propagate reaching tile -- if collision didn't occure on tile reach
	if (event.pos != event.reset_to) {
		MoveEvent move_event;
		move_event.actor = data.id;
		move_event.source = event.pos;
		move_event.target = data.target;
		move_event.type = MoveEvent::Reached;
		context.move_sender.send(move_event);
	}
}

MoveStyle getMoveStyle(MovementData const & data) {
	std::size_t n{0u};
	auto tmp = data.move;
	while (tmp != data.look) {
		tmp = rotate(tmp, true);
		++n;
		ASSERT(n < 8u);
	}
	if (n == 0u) {
		return MoveStyle::Forward;
	} else if (n == 3u || n == 4u || n == 5u) {
		return MoveStyle::Backward;
	} else {
		return MoveStyle::Sideward;
	}
}

float calcSpeedFactor(MovementData const& data) {
	// calculate speed_factor
	float speed_factor = 1.f + movement_impl::DELTA_SPEEDFACTOR * data.num_speed_boni;
	
	// consider movement style
	auto style = getMoveStyle(data);
	switch (style) {
		case movement_impl::MoveStyle::Backward:
			speed_factor *= movement_impl::BACKWARD_SPEEDFACTOR;
			break;
			
		case movement_impl::MoveStyle::Sideward:
			speed_factor *= movement_impl::SIDEWARD_SPEEDFACTOR;
			break;
			
		case movement_impl::MoveStyle::Forward:
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

	if (data.move == sf::Vector2i{}) {
		// leave tile
		moveToTarget(context, data);
	}

	// interpolate movement
	float delta{1.f};
	if (data.move != sf::Vector2i{}) {
		delta = data.max_speed * calcSpeedFactor(data) * elapsed.asSeconds();
	}
	auto step = data.pos + delta * sf::Vector2f{data.move};

	// check whether target was reached
	bool reached = false;
	auto target = sf::Vector2f{data.target};
	float old_dist = utils::distance(data.pos, target);
	float new_dist = utils::distance(step, target);
	if (new_dist <= movement_impl::MOVEMENT_ACCURACY || new_dist > old_dist) {
		// target was nearly reached or exceeded
		step = target;
		reached = true;
	}
	data.last_pos = data.pos;
	data.pos = step;
	data.has_changed = true;

	if (reached) {
		// propagate tile reached
		MoveEvent event;
		event.actor = data.id;
		event.source = sf::Vector2u{sf::Vector2i{target} - data.move};
		event.target = data.target;
		event.type = MoveEvent::Reached;
		context.move_sender.send(event);

		// stop movement
		data.move = sf::Vector2i{};
	}
	
	//data.is_moving = data.move != sf::Vector2i{};
}

}  // ::movement_impl

// ---------------------------------------------------------------------------

MovementSystem::MovementSystem(LogContext& log, std::size_t max_objects, DungeonSystem& dungeon)
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
	auto& data = query(event.actor);

	movement_impl::start(context, data, event);
}

void MovementSystem::handle(CollisionEvent const& event) {
	if (!has(event.actor)) {
		// object has no movement component
		// note: object might have been deleted
		return;
	}
	auto& data = query(event.actor);

	movement_impl::stop(context, data, event);
}

void MovementSystem::update(sf::Time const& elapsed) {
	dispatch<InputEvent>(*this);
	dispatch<CollisionEvent>(*this);

	updateRange(context, begin(), end(), elapsed);

	propagate<MoveEvent>();
}

}  // ::core

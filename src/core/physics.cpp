#include <Thor/Vectors.hpp>

#include <utils/assert.hpp>
#include <utils/algorithm.hpp>

#include <core/algorithm.hpp>
#include <core/physics.hpp>

namespace core {

namespace physics_impl {

float const MOVEMENT_VELOCITY = 0.0001f;
float const MIN_SPEEDFACTOR = 0.25f;
float const MAX_SPEEDFACTOR = 1.75f;
float const DELTA_SPEEDFACTOR = 0.05f;
float const SIDEWARD_SPEEDFACTOR = 0.9f;
float const BACKWARD_SPEEDFACTOR = 0.75f;

float const MAX_COLLISION_RADIUS = 1.f;

// to determine whether an object is centered on a cell or not
float const CELL_CENTER_DIVERGENCE = 0.00001f;

Context::Context(LogContext& log, CollisionSender& collision_sender, FocusSender& focus_sender, PhysicsManager& physics_manager, DungeonSystem& dungeon_system)
	: log{log}
	, collision_sender{collision_sender}
	, focus_sender{focus_sender}
	, physics_manager{physics_manager}
	, dungeon_system{dungeon_system} {}

// ---------------------------------------------------------------------------

void updateRange(Context& context, PhysicsManager::iterator begin,
	PhysicsManager::iterator end, sf::Time const& elapsed) {
	for (auto i = begin; i != end; ++i) {
		auto& data = *i;
		if (data.move != sf::Vector2f{}) {
			physics_impl::interpolate(context, data, elapsed);
		}
	}
}

void start(Context& context, PhysicsData& data, InputEvent const& event) {
	// apply movement direction
	data.move = event.move;
	
	// apply facing direction
	if (event.look.x != 0 || event.look.y != 0) {
		data.face = event.look;
		data.has_changed = true;
	}
}

MoveStyle getMoveStyle(PhysicsData const & data) {
	ASSERT(data.move.x != 0 || data.move.y != 0);
	
	float angle = std::abs(thor::signedAngle(data.move, data.face));
	
	if (angle < 45.0) {
		return MoveStyle::Forward;
	} else if (angle < 225.0) {
		return MoveStyle::Sideward;
	} else {
		return MoveStyle::Backward;
	}
}

float calcSpeedFactor(PhysicsData const& data) {
	// calculate speed_factor
	float speed_factor = 1.f + physics_impl::DELTA_SPEEDFACTOR * data.num_speed_boni;
	
	// consider movement style
	auto style = getMoveStyle(data);
	switch (style) {
		case physics_impl::MoveStyle::Backward:
			speed_factor *= physics_impl::BACKWARD_SPEEDFACTOR;
			break;
			
		case physics_impl::MoveStyle::Sideward:
			speed_factor *= physics_impl::SIDEWARD_SPEEDFACTOR;
			break;
			
		case physics_impl::MoveStyle::Forward:
			// no change
			break;
	}
	
	if (speed_factor < physics_impl::MIN_SPEEDFACTOR) {
		speed_factor = physics_impl::MIN_SPEEDFACTOR;
	} else if (speed_factor > physics_impl::MAX_SPEEDFACTOR) {
		speed_factor = physics_impl::MAX_SPEEDFACTOR;
	}
	ASSERT(speed_factor >= physics_impl::MIN_SPEEDFACTOR);
	ASSERT(speed_factor <= physics_impl::MAX_SPEEDFACTOR);
	
	return speed_factor;
}

bool checkCollision(Context& context, PhysicsData const & data, sf::Vector2f const & pos) {
	CollisionEvent event;
	event.actor = data.id;
	ASSERT(context.physics_manager.has(data.id));
	
	// determine grab target cell
	sf::Vector2u tile_pos{pos};
	
	if (data.scene == 0u) {
		// object already vanished -- nothing to collide with
		return true;
	}

	ASSERT(context.dungeon_system[data.scene].has(tile_pos));
	auto& cell = context.dungeon_system[data.scene].getCell(tile_pos);

	if (cell.terrain != Terrain::Floor) {
		// terrain collision detected!
		event.collider = 0u;
		event.pos = pos;
		return false;
	}
	
	
	
	// tba: add object collision!!
	return true;
}

void interpolate(Context& context, PhysicsData& data, sf::Time const& elapsed) {
	if (data.scene == 0u) {
		// object already vanished
		return;
	}
	ASSERT(data.max_speed >= 0.f);
	ASSERT(data.max_speed <= physics_impl::MAX_SPEED);
	
	// interpolate target
	float delta{1.f};
	if (data.move != sf::Vector2f{}) {
		delta = data.max_speed * calcSpeedFactor(data) *
			physics_impl::MOVEMENT_VELOCITY * elapsed.asMilliseconds();
	}
	auto target = data.pos + delta * data.move;
	
	// collision-check target
	if (!checkCollision(context, data, target)) {
		// stop movement
		data.move = sf::Vector2f{};
		
	} else {
		sf::Vector2u from{data.pos};
		sf::Vector2u to{target};
		if (from != to) {
			// update collision map
			auto& dungeon = context.dungeon_system[data.scene];
			ASSERT(dungeon.has(from));
			ASSERT(dungeon.has(to));
			auto& src = dungeon.getCell(from);
			auto& dst = dungeon.getCell(to);
			
			utils::pop(src.entities, data.id);
			dst.entities.push_back(data.id);
		}
		
		// apply movement
		data.pos = target;
	}
}

}  // ::physics_impl

// ---------------------------------------------------------------------------

PhysicsSystem::PhysicsSystem(LogContext& log, std::size_t max_objects, DungeonSystem& dungeon)
	// Event API
	: utils::EventListener<InputEvent>{}
	, utils::EventSender<CollisionEvent, FocusEvent>{}
	, PhysicsManager{max_objects}
	, context{log, *this, *this, *this, dungeon} {}

void PhysicsSystem::handle(InputEvent const& event) {
	if (!has(event.actor)) {
		// object has no movement component
		// note: object might have been deleted
		return;
	}
	auto& data = query(event.actor);

	physics_impl::start(context, data, event);
}

void PhysicsSystem::update(sf::Time const& elapsed) {
	dispatch<InputEvent>(*this);

	updateRange(context, begin(), end(), elapsed);
	
	propagate<CollisionEvent>();
	propagate<FocusEvent>();
}

}  // ::core

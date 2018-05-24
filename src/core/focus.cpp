#include <limits>

#include <utils/assert.hpp>
#include <utils/algorithm.hpp>

#include <core/focus.hpp>

namespace core {

float const MAX_SIGHT = 25.f;

// ---------------------------------------------------------------------------

namespace focus_impl {

FovQuery::FovQuery(FocusData const & actor_focus, MovementData const & actor_move, MovementManager const & move_manager, FocusManager const & focus_manager)
	: move_manager{move_manager}
	, focus_manager{focus_manager}
	, actor_move{actor_move}
	, actor_focus{actor_focus}
	, collider{}
	, focus{0u}
	, best_value{std::numeric_limits<float>::max()} {
	collider.radius = actor_focus.sight;
}

sf::IntRect FovQuery::getRange() const {
	return utils::toIntRect(actor_move.pos, collider.radius);
}

void FovQuery::operator()(sf::Vector2f const & pos, std::vector<ObjectID> const & cell) {
	for (auto id: cell) {
		if (id == actor_move.id) {
			// ignore actor
			continue;
		}
		auto const & move_data = move_manager.query(id);
		if (!focus_manager.has(id)) {
			// ignore objects without focus data
			continue;
		}
		auto const & focus_data = focus_manager.query(id);
		if (!focus_data.is_active) {
			// inactive entities
			continue;
		}
		auto value = utils::evalPos(actor_move.pos, actor_move.look, actor_focus.fov, actor_focus.sight, move_data.pos);
		if (value > -1.f && value < best_value) {
			focus = id;
			best_value = value;
		}
	}
}

// ---------------------------------------------------------------------------

ObjectID getFocus(ObjectID actor, Dungeon const & dungeon,
	FocusManager const & focus_manager, MovementManager const & movement_manager) {
	
	FovQuery fov_query{focus_manager.query(actor), movement_manager.query(actor), movement_manager, focus_manager};
	
	dungeon.traverse(fov_query);
	return fov_query.focus;
}

}  // ::focus_impl

// ---------------------------------------------------------------------------

FocusSystem::FocusSystem(LogContext& log, std::size_t max_objects)
	// Component API
	: FocusManager{max_objects} {
}

void FocusSystem::update(sf::Time const& elapsed) {
	/// @note focus stuff is manually triggered by other systems,
	///		so this system is only used for focus container purpose
}

}  // ::core

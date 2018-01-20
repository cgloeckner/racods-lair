#include <limits>

#include <utils/assert.hpp>
#include <utils/algorithm.hpp>

#include <core/focus.hpp>

namespace core {

float const MAX_SIGHT = 25.f;

// ---------------------------------------------------------------------------

namespace focus_impl {

Context::Context(LogContext& log, FocusSender& focus_sender,
	FocusManager& focus_manager, DungeonSystem& dungeon_system,
	MovementManager const& movement_manager)
	: log{log}
	, focus_sender{focus_sender}
	, focus_manager{focus_manager}
	, dungeon_system{dungeon_system}
	, movement_manager{movement_manager} {}

// ---------------------------------------------------------------------------

FovQuery::FovQuery(FocusData const & actor_focus, MovementData const & actor_move, MovementManager const & move_manager)
	: move_manager{move_manager}
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
		auto value = utils::evalPos(actor_move.pos, sf::Vector2f{actor_move.look}, actor_focus.fov, actor_focus.sight, move_data.pos);
		if (value > -1.f && value < best_value) {
			focus = id;
			best_value = value;
		}
	}
}

// ---------------------------------------------------------------------------

ObjectID getFocus(ObjectID actor, Dungeon const & dungeon,
	FocusManager const & focus_manager, MovementManager const & movement_manager) {
	
	FovQuery fov_query{focus_manager.query(actor), movement_manager.query(actor), movement_manager};
	
	dungeon.traverse(fov_query);
	return fov_query.focus;
}

}  // ::focus_impl

// ---------------------------------------------------------------------------

FocusSystem::FocusSystem(LogContext& log, std::size_t max_objects, DungeonSystem& dungeon,
	MovementManager const& movement_manager)
	// Event API
	: utils::EventListener<InputEvent, MoveEvent>{}
	, utils::EventSender<FocusEvent>{}  // Component API
	, FocusManager{max_objects}
	, context{log, *this, *this, dungeon, movement_manager} {}

void FocusSystem::handle(InputEvent const& event) {
}

void FocusSystem::handle(MoveEvent const& event) {
}

void FocusSystem::update(sf::Time const& elapsed) {
	dispatch<MoveEvent>(*this);
	dispatch<InputEvent>(*this);

	propagate<FocusEvent>();
}

}  // ::core

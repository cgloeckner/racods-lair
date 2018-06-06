#include <utils/enum_utils.hpp>
#include <core/algorithm.hpp>
#include <core/collision.hpp>
#include <core/movement.hpp>
#include <rpg/input.hpp>

namespace rpg {

namespace input_impl {

unsigned int const TOGGLE_COOLDOWN   = 250u;
unsigned int const SLIDE_COOLDOWN_MS = 200u;

Context::Context(core::LogContext& log, core::InputSender& input_sender,
	ActionSender& action_sender, core::DungeonSystem const& dungeon,
	core::MovementManager const& movement, core::CollisionManager const & collision,
	core::FocusManager const& focus)
	: log{log}
	, input_sender{input_sender}
	, action_sender{action_sender}
	, dungeon{dungeon}
	, movement{movement}
	, collision{collision}
	, focus{focus} {
	// register player actions as gameplay actions in priority order
	gameplay_actions = {PlayerAction::Pause, PlayerAction::Attack,
		PlayerAction::Interact, PlayerAction::UseSlot, PlayerAction::PrevSlot,
		PlayerAction::NextSlot};
}

// ---------------------------------------------------------------------------

bool isActive(
	Context const& context, InputData const& data, PlayerAction action) {
	return context.mapper.isActive(data.keys.get(action));
}

void queryInput(Context const& context, InputData& data,
	core::InputEvent& input, ActionEvent& action) {
	// setup movement direction
	if (isActive(context, data, PlayerAction::MoveN)) {
		input.move.y = -1;
	} else if (isActive(context, data, PlayerAction::MoveS)) {
		input.move.y = 1;
	}
	if (isActive(context, data, PlayerAction::MoveW)) {
		input.move.x = -1;
	} else if (isActive(context, data, PlayerAction::MoveE)) {
		input.move.x = 1;
	}

	// setup looking direction
	if (isActive(context, data, PlayerAction::LookN)) {
		input.look.y = -1;
	} else if (isActive(context, data, PlayerAction::LookS)) {
		input.look.y = 1;
	}
	if (isActive(context, data, PlayerAction::LookW)) {
		input.look.x = -1;
	} else if (isActive(context, data, PlayerAction::LookE)) {
		input.look.x = 1;
	}
	if (isActive(context, data, PlayerAction::ToggleAutoLook)) {
		// toggle auto look
		if (data.cooldown == sf::Time::Zero) {
			context.log.debug << "[Rpg/Input] " << "toggled auto-look\n";
			data.auto_look = !data.auto_look;
			data.cooldown = sf::milliseconds(input_impl::TOGGLE_COOLDOWN);
		}
	}
	if (input.move != sf::Vector2f{} && input.look != sf::Vector2f{}) {
		// disable auto looking if strifing is explicitly given
		data.auto_look = false;
	}
	if (input.move != sf::Vector2f{} && data.auto_look) {
		// ignore looking keys while moving with auto look
		input.look = input.move;
	}
	if (input.look == sf::Vector2f{}) {
		// use previous looking
		input.look = context.movement.query(data.id).look;
	}

	// setup player action
	for (auto tmp : context.gameplay_actions) {
		if (isActive(context, data, tmp)) {
			action.action = tmp;
			break;
		}
	}
}

void updateInput(Context& context, InputData& data, sf::Time const& elapsed) {
	// cool down auto-look toggle
	data.cooldown -= elapsed;
	if (data.cooldown < sf::Time::Zero) {
		data.cooldown = sf::Time::Zero;
	}
	// cool down sliding
	data.slide_cd -= elapsed;
	if (data.slide_cd < sf::Time::Zero) {
		data.slide_cd = sf::Time::Zero;
	}

	// query actual gameplay action
	core::InputEvent input_event;
	ActionEvent action_event;
	input_event.actor = data.id;
	action_event.actor = data.id;
	queryInput(context, data, input_event, action_event);
	
	if (data.slide_cd > sf::Time::Zero) {
		// drop movement input
		input_event.move = context.movement.query(data.id).move;
	}
	
	// adjust movement
	/// @note This should be done here to force adjustment
	/// of the next input vector by the player (which would
	/// reset the adjustment after a couple of ms).
	if (adjustMovement(context, data, input_event.move)) {
		data.slide_cd = sf::milliseconds(SLIDE_COOLDOWN_MS);
	}

	if (data.is_active) {
		// propagate input event
		context.input_sender.send(input_event);
	}

	if (data.is_active || action_event.action == PlayerAction::Pause) {
		// propagate action event
		context.action_sender.send(action_event);
	}
}

bool adjustMovement(Context const& context, InputData const& data, sf::Vector2f& vector) {
	if (vector == sf::Vector2f{}) {
		return false;
	}

	auto const& move_data = context.movement.query(data.id);
	ASSERT(move_data.scene > 0u);
	auto const& dungeon = context.dungeon[move_data.scene];

	auto canAccess = [&](sf::Vector2f const& move) {
		// create fake movement data with predicted position
		auto mv_cpy = move_data;
		mv_cpy.move = move;
		auto delta = core::movement_impl::getSpeedDelta(mv_cpy, sf::milliseconds(core::MAX_FRAMETIME_MS));
		mv_cpy.pos += move * delta;
		
		core::CollisionResult coll;
		core::checkAnyCollision(context.movement, context.collision, dungeon, mv_cpy, coll);
		return !coll.meansCollision();
	};
	
	// check given movement
	if (canAccess(vector)) {
		// everything is ok (at least no tile collision was detected!)
		return false;
	}
	
	// try alternative direction (randomly chosen)
	auto right = core::rotate(vector, 45.f);
	auto left = core::rotate(vector, -45.f);
	sf::Vector2f decision, other;
	if (thor::random(0u, 1u) == 0u) {
		decision = right;
		other = left;
	} else {
		decision = left;
		other = right;
	}
	
	if (canAccess(decision)) {
		// use this movement vector
		vector = decision;
		return true;
	} else if (canAccess(other)) {
		// use other movement vector
		vector = other;
		return true;
	} else {
		// cannot move, clear vector!
		vector = sf::Vector2f{};
		return false;
	}
}

void onDeath(InputData& data) { data.is_active = false; }

void onSpawn(InputData& data) { data.is_active = true; }

}  // ::input_impl

// ---------------------------------------------------------------------------

InputSystem::InputSystem(core::LogContext& log, std::size_t max_objects,
	core::DungeonSystem const& dungeon, core::MovementManager const& movement,
	core::CollisionManager const & collision, core::FocusManager const& focus)
	: utils::EventListener<DeathEvent, SpawnEvent>{}
	, utils::EventSender<core::InputEvent, ActionEvent>{}
	, InputManager{max_objects}
	, context{log, *this, *this, dungeon, movement, collision, focus} {}

void InputSystem::reset() { context.mapper = utils::InputMapper{}; }

void InputSystem::handle(DeathEvent const& event) {
	if (!has(event.actor)) {
		return;
	}
	auto& data = query(event.actor);
	input_impl::onDeath(data);
}

void InputSystem::handle(SpawnEvent const& event) {
	if (!has(event.actor)) {
		return;
	}
	auto& data = query(event.actor);
	input_impl::onSpawn(data);
}

void InputSystem::handle(sf::Event const& event) {
	context.mapper.pushEvent(event);
}

void InputSystem::update(sf::Time const& elapsed) {
	dispatch<DeathEvent>(*this);
	dispatch<SpawnEvent>(*this);

	for (auto& data : *this) {
		input_impl::updateInput(context, data, elapsed);
	}

	propagate<core::InputEvent>();
	propagate<ActionEvent>();
}

}  // ::game

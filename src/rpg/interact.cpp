#include <utils/algorithm.hpp>
#include <core/collision.hpp>
#include <rpg/interact.hpp>

namespace rpg {

namespace interact_impl {

Context::Context(core::LogContext& log, core::InputSender& input_sender,
	ItemSender& item_sender, core::MovementManager const& movement,
	core::FocusManager const& focus, PlayerManager const& player)
	: log{log}
	, input_sender{input_sender}
	, item_sender{item_sender}
	, movement{movement}
	, focus{focus}
	, player{player} {}

// ---------------------------------------------------------------------------

void moveBarrier(Context& context, InteractData& data, core::ObjectID actor) {
	if (data.moving) {
		// already moving
		context.log.debug << "[Rpg/Interact] " << "Cannot move barrier: Already moving\n";
		return;
	}

	// check distance
	auto const& actor_move = context.movement.query(actor);
	auto const& target_move = context.movement.query(data.id);
	auto dist = utils::distance(actor_move.pos, target_move.pos);
	if (dist > MAX_DISTANCE || actor_move.scene != target_move.scene) {
		// too far away
		context.log.debug << "[Rpg/Interact] " << "Cannot move barrier: Too far away\n";
		return;
	}

	// determine movement direction
	auto dir = actor_move.move;
	if (dir == sf::Vector2i{}) {
		auto const& actor_focus = context.focus.query(actor);
		dir = actor_focus.look;
	}

	// trigger movement
	core::InputEvent event;
	event.actor = data.id;
	event.move = dir;
	context.input_sender.send(event);

	// data.moving = true;
}

void stopBarrier(Context& context, InteractData& data) {
	if (!data.moving) {
		// not moving, yet
		return;
	}

	// trigger movement stop
	core::InputEvent event;
	event.actor = data.id;
	event.move = {};
	context.input_sender.send(event);

	data.moving = false;
}

void lootCorpse(Context& context, InteractData& data, core::ObjectID actor) {
	if (!context.player.has(actor)) {
		// only players can loot
		return;
	}
	auto const& player = context.player.query(actor);
	ASSERT(player.player_id > 0u)

	// check for loot
	ASSERT(player.player_id <= data.loot.size());
	auto& loot = data.loot[player.player_id - 1u];
	if (loot.empty()) {
		// nothing to loo
		return;
	}

	// grab loot
	ItemEvent event;
	event.actor = actor;
	event.type = ItemEvent::Add;
	for (auto& node : loot) {
		event.item = node.item;
		event.quantity = node.quantity;
		context.item_sender.send(event);
	}
	loot.clear();
}

void onInteract(Context& context, InteractData& data, core::ObjectID actor) {
	switch (data.type) {
		case InteractType::Barrier:
			moveBarrier(context, data, actor);
			break;

		case InteractType::Corpse:
			lootCorpse(context, data, actor);
			break;
	}
}

void onTileLeft(Context& context, InteractData& data) {
	switch (data.type) {
		case InteractType::Barrier:
			data.moving = true;
			break;

		default:
			break;
	}
}

void onUpdate(Context& context, InteractData& data) {
	switch (data.type) {
		case InteractType::Barrier:
			if (data.moving) {
				stopBarrier(context, data);
			}
			break;

		default:
			break;
	}
}

}  // ::interact_impl

// ---------------------------------------------------------------------------

InteractSystem::InteractSystem(core::LogContext& log, std::size_t max_objects,
	core::MovementManager const& movement, core::FocusManager const& focus,
	PlayerManager const& player)
	: utils::EventListener<core::MoveEvent, InteractEvent>{}
	, utils::EventSender<core::InputEvent, ItemEvent>{}
	, InteractManager{max_objects}
	, context{log, *this, *this, movement, focus, player} {}

void InteractSystem::handle(core::MoveEvent const& event) {
	if (!has(event.actor)) {
		// object has no interact component
		return;
	}
	auto& data = query(event.actor);

	switch (event.type) {
		case core::MoveEvent::Left:
			interact_impl::onTileLeft(context, data);
			break;

		default:
			// others not handled here
			break;
	}
}

void InteractSystem::handle(InteractEvent const& event) {
	if (!has(event.target)) {
		// object has no interact component
		return;
	}
	auto& data = query(event.target);

	interact_impl::onInteract(context, data, event.actor);
}

void InteractSystem::update(sf::Time const& elapsed) {
	dispatch<core::MoveEvent>(*this);
	dispatch<InteractEvent>(*this);

	for (auto& data : *this) {
		onUpdate(context, data);
	}

	propagate<core::InputEvent>();
	propagate<ItemEvent>();
}

}  // ::game

#include <utils/enum_utils.hpp>
#include <core/algorithm.hpp>
#include <rpg/action.hpp>

namespace rpg {

namespace action_impl {

Context::Context(core::LogContext& log, core::InputSender& input_sender,
	core::AnimationSender& animation_sender, ActionSender& action_sender)
	: log{log}
	, input_sender{input_sender}
	, animation_sender{animation_sender}
	, action_sender{action_sender} {}

// ---------------------------------------------------------------------------

void onInput(Context& context, ActionData& data, core::InputEvent const& event) {
	// ignore move or looking input if actor is dead
	if (data.dead) {
		if (event.move != sf::Vector2i() || event.look != sf::Vector2i()) {
			return;
		}
	}

	//if (data.moving && event.move != sf::Vector2i{}) {
		// mark movement stop
		//data.moving = false;
	//}

	context.input_sender.send(event);
}

void onAnimation(
	Context& context, ActionData& data, core::AnimationEvent const& event) {
	if (event.type == core::AnimationEvent::Action) {
		data.idle = event.action == core::AnimationAction::Idle;
	}
}

void onMove(Context& context, ActionData& data, core::MoveEvent const& event) {
	/*if (event.type == core::MoveEvent::Left) {
		data.moving = true;
		// start leg animation
		core::AnimationEvent ani;
		ani.actor = data.id;
		ani.type = core::AnimationEvent::Move;
		ani.move = true;
		context.animation_sender.send(ani);

	} else if (event.type == core::MoveEvent::Reached) {
		// stop leg animation
		core::AnimationEvent ani;
		ani.actor = data.id;
		ani.type = core::AnimationEvent::Move;
		ani.move = false;
		context.animation_sender.send(ani);
	}*/
}

void onCollision(
	Context& context, ActionData& data, core::CollisionEvent const& event) {
	//data.moving = false;
	// note: collision never occures in the middle of a movement. so the
	// movement is either started or stopped. the corresponding move event
	// will handle the legs' animation
}

void onAction(Context& context, ActionData& data, ActionEvent const& event) {	
	if (data.dead && (event.action != PlayerAction::Pause)) {
		context.log.debug << "[Rpg/Action] " << "Ignored action '"
			<< rpg::to_string(event.action) << "' by dead actor\n";
		return;
	}

	// conditional forward
	switch (event.action) {
		case PlayerAction::Attack:
		case PlayerAction::Interact:
		case PlayerAction::UseSlot:
			if (data.idle) {
				if (event.action == PlayerAction::UseSlot) {
				}
				data.idle = false;
				context.action_sender.send(event);
			}
			break;

		default:
			context.action_sender.send(event);
	}
}

void onDeath(Context& context, ActionData& data, DeathEvent const& event) {
	data.dead = true;

	core::AnimationEvent ani_event;
	ani_event.force = true; // ignore whether flying or not
	ani_event.actor = data.id;
	/*
	// trigger stop legs' animation
	ani_event.type = core::AnimationEvent::Move;
	ani_event.move = false;
	context.animation_sender.send(ani_event);
	*/
	// trigger dying animation
	ani_event.type = core::AnimationEvent::Action;
	ani_event.action = core::AnimationAction::Die;
	context.animation_sender.send(ani_event);
}

void onSpawn(Context& context, ActionData& data, SpawnEvent const& event) {
	data.dead = false;
	
	// trigger idle animation
	core::AnimationEvent ani_event;
	ani_event.actor = data.id;
	ani_event.type = core::AnimationEvent::Action;
	ani_event.action = core::AnimationAction::Idle;
	context.animation_sender.send(ani_event);
}

void onFeedback(Context const & context, ActionData& actor, FeedbackEvent const& event) {
	if (actor.dead) {
		return;
	}
	
	switch (event.type) {
		case FeedbackType::ItemNotFound:
		case FeedbackType::CannotUseThis:
		case FeedbackType::EmptyShortcut:
		case FeedbackType::NotEnoughMana:
			// reset idle after failed (possibly quickuse-related) actions
			actor.idle = true;
			break;
		
		default:
			break;
	}
}

}  // ::action_impl

// ---------------------------------------------------------------------------

ActionSystem::ActionSystem(core::LogContext& log, std::size_t max_objects)
	: utils::EventListener<core::InputEvent, core::AnimationEvent,
		  core::MoveEvent, core::CollisionEvent, ActionEvent, DeathEvent,
		  SpawnEvent, FeedbackEvent>{}
	, utils::EventSender<core::InputEvent, core::AnimationEvent, ActionEvent>{}
	, ActionManager{max_objects}
	, context{log, *this, *this, *this} {}

void ActionSystem::handle(core::InputEvent const& event) {
	if (!has(event.actor)) {
		// no such component - but forward it!
		send(event);
		return;
	}
	auto& actor = query(event.actor);

	onInput(context, actor, event);
}

void ActionSystem::handle(core::AnimationEvent const& event) {
	if (!has(event.actor)) {
		// no such component
		return;
	}
	auto& actor = query(event.actor);

	onAnimation(context, actor, event);
}

void ActionSystem::handle(core::MoveEvent const& event) {
	if (!has(event.actor)) {
		// no such component
		return;
	}
	auto& actor = query(event.actor);

	onMove(context, actor, event);
}

void ActionSystem::handle(core::CollisionEvent const& event) {
	if (!has(event.actor)) {
		// no such component
		return;
	}
	auto& actor = query(event.actor);

	onCollision(context, actor, event);
}

void ActionSystem::handle(ActionEvent const& event) {
	if (!has(event.actor)) {
		// no such component - but forward it!
		send(event);
		return;
	}
	auto& actor = query(event.actor);

	onAction(context, actor, event);
}

void ActionSystem::handle(DeathEvent const& event) {
	if (!has(event.actor)) {
		// no such component
		return;
	}
	auto& actor = query(event.actor);

	onDeath(context, actor, event);
}

void ActionSystem::handle(SpawnEvent const& event) {
	if (!has(event.actor)) {
		// no such component
		return;
	}
	auto& actor = query(event.actor);

	onSpawn(context, actor, event);
}

void ActionSystem::handle(FeedbackEvent const& event) {
	if (!has(event.actor)) {
		// no such component
		return;
	}
	auto& actor = query(event.actor);

	onFeedback(context, actor, event);
}

void ActionSystem::update(sf::Time const& elapsed) {
	dispatch<SpawnEvent>(*this);
	dispatch<core::InputEvent>(*this);
	dispatch<core::AnimationEvent>(*this);
	dispatch<core::MoveEvent>(*this);
	dispatch<core::CollisionEvent>(*this);
	dispatch<ActionEvent>(*this);
	dispatch<DeathEvent>(*this);
	dispatch<SpawnEvent>(*this);
	dispatch<FeedbackEvent>(*this);

	propagate<core::InputEvent>();
	propagate<core::AnimationEvent>();
	propagate<ActionEvent>();
}

}  // ::game

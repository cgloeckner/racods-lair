#pragma once
#include <utils/input_mapper.hpp>
#include <core/dungeon.hpp>
#include <rpg/entity.hpp>
#include <rpg/event.hpp>

namespace rpg {

namespace action_impl {

/// Context of the input handling
struct Context {
	core::LogContext& log;
	core::InputSender& input_sender;
	core::AnimationSender& animation_sender;
	ActionSender& action_sender;

	Context(core::LogContext& log, core::InputSender& input_sender,
		core::AnimationSender& animation_sender, ActionSender& action_sender);
};

// ---------------------------------------------------------------------------

/// Handle incomming input
/**
 *	Handles incomming input events. If the actor cannot comply (e.g. because
 *	he is dead), the event is ignored.
 *
 *	@param context Action context to use
 *	@param actor ActionData of the actor
 *	@param event Received InputEvent
 */
void onInput(
	Context& context, ActionData& actor, core::InputEvent const& event);

/// Handle incomming animation event
/**
 *	Handle incomming animation events to know whether an object is idling or
 *not.
 *
 *	@param context Action context to use
 *	@param actor ActionData of the actor
 *	@param event Received AnimationEvent
 */
void onAnimation(
	Context& context, ActionData& actor, core::AnimationEvent const& event);

/// Handle incomming move event
/**
 *	Handle incomming move events to know whether an object is moving. The
 *	events are usually received from the physics systems. The object is just
 *	started if it was not moving before but left a tile. It is stopped if it
 *	reached a tile and no further input was previously triggered.
 *	Once an object started its movement, a suitable animation is triggered.
 *
 *	@param context Action context to use
 *	@param actor ActionData of the actor
 *	@param event Received MoveEvent
 */
void onMove(Context& context, ActionData& actor, core::MoveEvent const& event);

/// Handle incomming collision event
/**
 *	Handle incomming collision events to know whether an object is stopped by
 *	a collision. No animation events are propagates here.
 *
 *	@param context Action context to use
 *	@param actor ActionData of the actor
 *	@param event Received CollisionEvent
 */
void onCollision(
	Context& context, ActionData& actor, core::CollisionEvent const& event);

/// Handle incomming action event
/**
 *	Handle incomming action events that specify the actor's next action. Those
 *	actions are only complied if the actor is able to perform an action. If
 *	complied the action is forwarded. If a direct action (like attacking, but
 *	not like using a quickslot shortcut) is propagated, the actor is marked as
 *	not idling.
 *
 *	@param context Action context to use
 *	@param actor ActionData of the actor
 *	@param event Received ActionEvent
 */
void onAction(Context& context, ActionData& actor, ActionEvent const& event);

/// Handle incomming death event
/**
 *	Handle incomming death event to stop movement and action animations.
 *
 *	@param context Action context to use
 *	@param actor ActionData of the actor
 *	@param event Received DeathEvent
 */
void onDeath(Context& context, ActionData& actor, DeathEvent const& event);

/// Handle incomming respawn event
/**
 *	Notify about respawn
 *
 *	@param context Action context to use
 *	@param actor ActionData of the actor
 *	@param event Received SpawnEvent
 */
void onSpawn(Context& context, ActionData& actor, SpawnEvent const& event);

/// Handle incomming feedback event
/// This is e.g. used to reset the state to idle after using an empty
/// quickslot.
/// @param context Action context to use
/// @param actor ActionData of the actor
/// @param event Received FeedbackEvent
void onFeedback(Context const & context, ActionData& actor, FeedbackEvent const& event);

}  // ::input_impl

// ---------------------------------------------------------------------------

/// The ActionSystem is used to control object behavior
/**
 *	InputEvents, which contain movement and looking, are forwarded to the
 *	physics system if they can be performed (e.g. object is not dead).
 *	AnimationEvents are handled to detect whether an action was finished.
 *	MoveEvents are received from the physics system to start movements.
 *	CollisionEvents are received from the physics system to stop movements.
 *	ActionEvents, which contain actions like attacking, are forwarded to the
 *	delay and/or quickslot system to perform them if they can be performed
 *	(e.g. actor is not dead).
 *	DeathEvents are received to detect objects' death.
 *	Actions are only forwarded. Once they are performed, the ActionSystem is
 *	notified via AnimationEvents about the current behavior. Movement is also
 *	forwarded only. Once a movement started, the ActionSystem is notified via
 *	MoveEvents to finally start the movement animation. In case of collisions
 *	or if the actor stopped its movement, the movement animation is also
 *	stopped.
 */
class ActionSystem
	// Event API
	: public utils::EventListener<core::InputEvent, core::AnimationEvent,
		  core::MoveEvent, core::CollisionEvent, ActionEvent, DeathEvent,
		  SpawnEvent, FeedbackEvent>,
	  public utils::EventSender<core::InputEvent, core::AnimationEvent,
		  ActionEvent>
	  // Component API
	  ,
	  public ActionManager {

  private:
	action_impl::Context context;

  public:
	ActionSystem(core::LogContext& log, std::size_t max_objects);

	void handle(core::InputEvent const& event);
	void handle(core::AnimationEvent const& event);
	void handle(core::MoveEvent const& event);
	void handle(core::CollisionEvent const& event);
	void handle(ActionEvent const& event);
	void handle(DeathEvent const& event);
	void handle(SpawnEvent const& event);
	void handle(FeedbackEvent const& event);

	void update(sf::Time const& elapsed);
};

}  // ::game

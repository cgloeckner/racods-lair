#pragma once
#include <core/event.hpp>
#include <rpg/entity.hpp>
#include <rpg/event.hpp>

namespace rpg {

namespace interact_impl {

/// Interaction context
struct Context {
	core::LogContext& log;
	core::InputSender& input_sender;
	ItemSender& item_sender;

	core::MovementManager const& movement;
	core::FocusManager const& focus;
	PlayerManager const& player;

	Context(core::LogContext& log, core::InputSender& input_sender,
		ItemSender& item_sender, core::MovementManager const& movement,
		core::FocusManager const& focus, PlayerManager const& player);
};

// ---------------------------------------------------------------------------

/// Move the barrier
/**
 *	This will move the barrier. If the actor is moving, the barrier will be
 *	moved into the same direciton. If he is not moving, the barrier will be
 *	moved into the actor's looking direction.
 *	Finally, this will propagate an InputEvent for the barrier in order to
 *	trigger the movement.
 *
 *	@param context Interact context to use
 *	@param data InteractData of the barrier
 *	@param actor ObjectID of the actor
 */
void moveBarrier(Context& context, InteractData& data, core::ObjectID actor);

/// Stop the barrier
/**
 *	This will stop the barrier's movement. It is usually called one frame after
 *	a barrier's movement was triggered. A non-moving InputEvent is propagated.
 *	If the barrier was not moving, nothing happens.
 *
 *	@param context Interact context to use
 *	@param data InteractData of the barrier
 */
void stopBarrier(Context& context, InteractData& data);

/// This will loot the corpse to the actor's inventory
/**
 *	This operation requires the actor to be a player. Otherwise nothing
 *	happens. Its loot slot within the given InteractData's corpse is looted
 *	and multiple ItemEvents are propagated about adding those items to his
 *	inventory. After that, the loot slot is cleared.
 *
 *	@param context Interact context to use
 *	@param data InteractData of the corpse
 *	@param actor ObjectID of the actor
 */
void lootCorpse(Context& context, InteractData& data, core::ObjectID actor);

/// This will trigger an interaction
/**
 *	The interaction between the actor and the specified InteractData is
 *	triggered here. Depending on the interactables type, a suitable function
 *	is invoked.
 *
 *	@param context Interact context to use
 *	@param data InteractData of the interactable
 *	@param actor ObjectID of the actor
 */
void onInteract(Context& context, InteractData& data, core::ObjectID actor);

/// This will start a barrier's movement
/**
 *	If the interactable is a barrier, it will be moved.
 *
 *	@param context Interact context to use
 *	@param data InteractData of the actor
 */
void onTileLeft(Context& context, InteractData& data);

/// This will stop a barrier's movement if moving
/**
 *	If the interactable is a barrier, it will be stopped.
 *
 *	@param context Interact context to use
 *	@param data InteractData of the actor
 */
void onUpdate(Context& context, InteractData& data);

}  // ::interact_impl

// ---------------------------------------------------------------------------

/// The InteractSystem is used to move barriers and loot corpses
/**
 *	InteractEvents are handled referring to the interactable's type. If it is
 *	a barrier, it is moved. An InputEvent is propagated for the barrier. If
 *	the interactable is a corpse and the actor is a player, it will be looted
 *	and ItemEvents are propagated per item type.
 */
class InteractSystem
	// Event API
	: public utils::EventListener<core::MoveEvent, InteractEvent>,
	  public utils::EventSender<core::InputEvent, ItemEvent>
	  // Component API
	  ,
	  public InteractManager {

  private:
	interact_impl::Context context;

  public:
	InteractSystem(core::LogContext& log, std::size_t max_objects, core::MovementManager const& movement,
		core::FocusManager const& focus, PlayerManager const& player);

	void handle(core::MoveEvent const& event);
	void handle(InteractEvent const& event);

	void update(sf::Time const& elapsed);
};

}  // ::game

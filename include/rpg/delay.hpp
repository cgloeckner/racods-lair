#pragma once
#include <utils/delay_system.hpp>
#include <core/dungeon.hpp>
#include <rpg/entity.hpp>
#include <rpg/event.hpp>

namespace rpg {

namespace delay_impl {

/// Delay context
struct Context {
	core::LogContext& log;
	core::AnimationSender& animation_sender;
	CombatSender& combat_sender;
	ProjectileSender& projectile_sender;
	InteractSender& interact_sender;

	core::DungeonSystem const& dungeon;
	core::MovementManager const& movement;
	core::FocusManager const& focus;
	core::AnimationManager const& animation;
	ItemManager const& item;
	StatsManager const& stats;
	InteractManager const& interact;
	PlayerManager const& player;

	utils::DelaySystem<CombatEvent> combats;
	utils::DelaySystem<ProjectileEvent> projectiles;
	utils::DelaySystem<InteractEvent> interacts;

	Context(core::LogContext& log, core::AnimationSender& animation_sender,
		CombatSender& combat_sender, ProjectileSender& projectile_sender,
		InteractSender& interact_sender, core::DungeonSystem const& dungeon,
		core::MovementManager const& movement, core::FocusManager const& focus,
		core::AnimationManager const& animation, ItemManager const& item,
		StatsManager const& stats, InteractManager const& interact,
		PlayerManager const& player);
};

// ---------------------------------------------------------------------------

/// Calculate delay duration based on animation
/// @param animation AnimationManager to query in
/// @param actor ObjectID of actor
/// @param action InputAction to determine delay for
/// @return delay time
sf::Time getDelayDuration(core::AnimationManager const& animation, core::ObjectID actor, core::AnimationAction action);

/// Query closest interaction target
core::ObjectID queryInteractable(Context& context, core::ObjectID actor);

/// Query closest melee target
core::ObjectID queryAttackable(Context& context, core::ObjectID actor);

/// Use the actor's weapon
/**
 *	This will handle attacking by weapons. The actor starts its attack
 *	animation (either melee or range, based on the weapons) immediately. The
 *	resulting Combat- or ProjectileEvent is delayed.
 *
 *	@param context Delay context to use
 *	@param actor ObjectID of the actor object
 */
void onAttack(Context& context, core::ObjectID actor);

/// Trigger the actor's interaction
/**
 *	This will handle the actor's interaction with another object. The actor
 *	starts its interact animation immediately. The resulting InteractionEvent
 *	is delayed. Which object is used as interaction target is also specified
 *	here. An object will always interact with an interactable from the next
 *	cell in focus direction.
 *
 *	@param context Delay context to use
 *	@param actor ObjectID of the actor object
 */
void onInteract(Context& context, core::ObjectID actor);

/// Trigger a perk use
/**
 *	This will handle the actor's perk usage. The actor starts its casting
 *	animation immediately. Either a Combat- or a ProjectileEvent is delayed.
 *	Mana consumption is not handled here, due to the incomming PerkEvent
 *	should came from the PerkSystem. So mana should already be handled here.
 *
 *	@param context Delay context to use
 *	@param actor ObjectID of the actor object
 *	@param perk PerkTemplate to use
 */
void onPerk(Context& context, core::ObjectID actor, PerkTemplate const& perk);

/// Update delays
/**
 *	This will update all outgoing events' remaining delay. Once an event
 *	reached zero, it will be propagated.
 *
 *	@param context Delay context to use
 *	@param elapsed Time since last update
 */
void onUpdate(Context& context, sf::Time const& elapsed);

}  // ::delay_impl

// ---------------------------------------------------------------------------

/// The DelaySystem is used to delay the execution of some events
/**
 *	Some events require to be executed with a delay. So e.g. using a bow
 *	should delay the projectile creation until some frames are played. So
 *	ActionEvents (like interaction or attacks) and PerkEvents (like casting a
 *	fireball) can be delayed. Those animations are handled here directly, so
 *	different kinds of AnimationEvents, CombatEvents, ProjectileEvents and
 *	InteractEvents are triggered. In fact, all incomming events are processed
 *	immediately. Their animations also triggered immediately. But all other
 *	resulting events like CombatEvents, ProjectileEvents and InteractEvents
 *	are delayed in execution. The default delay is 75% of the corresponding
 *	animation's total duration.
 *	The delay system does not hold any object-specific data.
 */
class DelaySystem : public utils::EventListener<ActionEvent, PerkEvent>,
					public utils::EventSender<core::AnimationEvent, CombatEvent,
						ProjectileEvent, InteractEvent> {

  private:
	delay_impl::Context context;

  public:
	DelaySystem(core::LogContext& log, core::DungeonSystem const& dungeon,
		core::MovementManager const& movement, core::FocusManager const& focus,
		core::AnimationManager const& animation, ItemManager const& item,
		StatsManager const& stats, InteractManager const& interact,
		PlayerManager const& player);

	void reset();

	void handle(ActionEvent const& event);
	void handle(PerkEvent const& event);
	void handle(ItemEvent const& event);

	void update(sf::Time const& elapsed);
};

}  // ::game

#pragma once
#include <rpg/entity.hpp>
#include <rpg/event.hpp>

namespace rpg {

namespace effect_impl {

/// Delay for applying the effects' damage, recovery etc.
extern unsigned int const MIN_ELAPSED_TIME;

/// Effect context
struct Context {
	core::LogContext& log;
	BoniSender& boni_sender;
	CombatSender& combat_sender;
	EffectSender& effect_sender;

	Context(core::LogContext& log, BoniSender& boni_sender,
		CombatSender& combat_sender, EffectSender& effect_sender);
};

// ---------------------------------------------------------------------------

/// Add an effect to an object
/**
 *	This adds the given effect to the actor. The effect's default duration is
 *	used to initialize the effect's remaining time. If no such effect exists
 *	yet, a new effect is added to the actor. If the effect is already
 *	inflicted, its remaining time is reset to the default duration. Effects
 *	are either volatile (they will disappear after some time) or perpetual
 *	(they only appear on death).
 *
 *	@param context Effect context to use
 *	@param actor EffectData of the actor
 *	@param effect EffectTemplate of the effect which should be added
 */
void addEffect(
	Context& context, EffectData& actor, EffectTemplate const& effect);

/// Remove an effect from an object
/**
 *	This removes the given effect from the actor, despite the effect is
 *	perpetual or not. If the given effect is not present, nothing happens.
 *
 *	@param context Effect context to use
 *	@param actor EffectData of the actor
 *	@param effect EffectTemplate of the effect which should be removed
 */
void removeEffect(
	Context& context, EffectData& actor, EffectTemplate const& effect);

/// Callback on object death
/**
 *	This is called as the event system is notified about an object's death.
 *	All active effects (despite perpetual or not) are immediately removed.
 *
 *	@param context Effect context to use
 *	@param actor EffectData of the actor
 */
void onDeath(Context& context, EffectData& actor);

// ---------------------------------------------------------------------------

/// Handle all effects
/**
 *	This handles all effects of the given actor. CombatEvents will be produced
 *	and propagated in order to apply the actual effects to the actor. All
 *	volatile effects are cooled down. Once a volatile effect is cooled down
 *	completely, it will be removed.
 *
 *	@param context Effect context to use
 *	@param actor EffectData of the actor
 *	@param step Time since last update (used to cool down)
 */
void handleEffects(Context& context, EffectData& actor, sf::Time const& step);

/// Updates all effects
/**
 *	This will check whether enough time elapsed for the next cycle of effect
 *	handling. If yes, `handleEffects()` is invoked.
 *
 *	@param context Effect context to use
 *	@param actor EffectData of the actor
 *	@param step Time since last update
 */
void onUpdate(Context& context, EffectData& actor, sf::Time const& elapsed);

}  // ::effect_impl

// ---------------------------------------------------------------------------

/// The EffectSystem manages combat effects
/**
 *	Each object can have multiple active effects. Those effects can manipulate
 *	stats temporary or over time. Effects can vanish over time (volatile
 *	effects) or remain until death (perpetual effects).
 *	EffectEvents are handled to inflict or remove effects. Effects do not
 *	stack, so reinflicting intoxication will reset the initial poison's
 *	remaining duration. On effects added or removed, BoniEvent are populated
 *	to update the effects' temporary stat boni or mali. Once per cycle
 *	(see MIN_ELAPSED_TIME) each active effects triggers a CombatEvent to apply
 *	damage and/or recovery to the actor.
 *	DeathEvents are handled to remove all events when an object died.
 *	EffectEvents are propagated once a volatile effect faded.
 */
class EffectSystem
	// Event API
	: public utils::EventListener<EffectEvent, DeathEvent>,
	  public utils::EventSender<BoniEvent, CombatEvent, EffectEvent>
	  // Component API
	  ,
	  public EffectManager {

  private:
	effect_impl::Context context;

  public:
	EffectSystem(core::LogContext& log, std::size_t max_objects);

	void handle(EffectEvent const& event);
	void handle(DeathEvent const& event);

	void update(sf::Time const& elapsed);
};

}  // ::game

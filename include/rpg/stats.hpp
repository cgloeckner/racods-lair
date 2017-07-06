#pragma once
#include <rpg/entity.hpp>
#include <rpg/event.hpp>

namespace rpg {

namespace stats_impl {

/// Context of the StatsSystem
struct Context {
	core::LogContext& log;
	StatsSender& stats_sender;
	DeathSender& death_sender;

	Context(core::LogContext& log, StatsSender& stats_sender,
		DeathSender& death_sender);
};

/// This will limit the given value in place
/**
 *	@post value in [0, max]
 *
 *	@param value Value that is ajusted in place
 *	@param max Maximum value specifying the valid range
 */
void cap(unsigned int& value, unsigned int max);

/// This applies the given stats event
/**
 *	This applies the given stats event to the actor. If the actor is already
 *	dead, nothing will be applied (e.g. no mana regeneration while death).
 *	All values are automatically limited to [0, max], where `max` is the
 *	stat's corresponding maximum value, specified by the actor's properties.
 *	If the minimum or maximum is exceeded, the given event's stats delta is
 *	adjusted to the actual applied difference.
 *	If the actor died because of this stats event, a death event is
 *	propagated. The death event will also notify about the causer of the stats
 *	event (e.g. the attacker whose bullet hit the actor), in order to let e.g.
 *	die AI know who killed him.
 *	If some stats really changed, true is returned - otherwise false.
 *
 *	@param context StatsContext to use
 *	@param actor StatsData to change
 *	@param event StatsEvent to use for the change
 *	@return true if something changed
 */
bool applyStats(Context& context, StatsData& actor, StatsEvent& event);

// ---------------------------------------------------------------------------

/// This will refresh the actor's depending values
/**
 *	The actor's base properties depend on the current level and attributes.
 *	The actual property levels are affected by all property boni, that are
 *	currently active due to equipped items, active effects (like being frozen)
 *	and other things.
 *
 *	@param actor StatsData to refresh
 */
void refresh(StatsData& actor);

/// This will add some stats boni to the actor
/**
 *	Stats boni may contain property boni/mali and defense boni/mali. Those
 *	are added to the current boni and the actor's state is recalculated.
 *
 *	@param context StatsContext to use
 *	@param actor StatsData to update
 *	@param boni StatsBoni to add
 */
void addBoni(Context& context, StatsData& actor, StatsBoni const& boni);

/// This will remove some stats boni from the actor
/**
 *	Stats boni may contain property boni/mali and defense boni/mali. Those
 *	are removed from the current boni and the actor's state is recalculated.
 *
 *	@param context StatsContext to use
 *	@param actor StatsData to update
 *	@param boni StatsBoni to remove
 */
void removeBoni(Context& context, StatsData& actor, StatsBoni const& boni);

/// This will increase the specified attribute by 1
/**
 *	This is e.g. used if a player applies an attribute point which is got for
 *	a levelup. The specified attribute's value is increased by one and the
 *	actor's state is recalculated.
 *
 *	@param context StatsContext to use
 *	@param actor StatsData to update
 *	@param attrib Attribute that should be increased
 */
void increaseAttribute(Context& context, StatsData& actor, Attribute attrib);

/// This will handle a levelup
/// The actor's level is increased by one and it's state is recalculated.
/// @param context StatsContext to use
/// @param actor StatsData to update
/// @param delta Number of levels raised (default: 1)
void onLevelup(Context& context, StatsData& actor, unsigned int delta=1u);

}  // ::stats_impl

// ---------------------------------------------------------------------------

/// The StatsSystem keeps track of all objects' stats.
/**
 *	Stats can be changed by StatsEvents. Applying a stats event will try to
 *	apply the specified changes and forward the event with slightly corrected
 *	values. If the object died during the update, a DeathEvent will be
 *	propagated.
 *	BoniEvents are used to add or remove property and defense boni/mali by
 *	equipped items or active effects. Those boni affect other stats.
 *	TrainingEvents are used to increase specific attributes. This is triggered
 *	by the PlayerSystem after decreasing the attribute points. Those attribute
 *	changes affect other stats.
 *	ExpEvents are used to notify about levelups, but not every ExpEvent
 *	causes. Changing the level affects other stats.
 *	Once an event was received that affects other stats, all depending stats
 *	are recalculated.
 */
class StatsSystem
	// Event API
	: public utils::EventListener<StatsEvent, BoniEvent, TrainingEvent,
		  ExpEvent>,
	  public utils::EventSender<StatsEvent, DeathEvent>
	  // Component API
	  ,
	  public StatsManager {

  private:
	stats_impl::Context context;

  public:
	StatsSystem(core::LogContext& log, std::size_t max_objects);

	void handle(StatsEvent const& event);
	void handle(BoniEvent const& event);
	void handle(TrainingEvent const& event);
	void handle(ExpEvent const& event);

	void update(sf::Time const& elapsed);
};

}  // ::game

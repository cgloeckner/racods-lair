#pragma once
#include <rpg/entity.hpp>
#include <rpg/event.hpp>

namespace rpg {

namespace perk_impl {

/// Perk context
struct Context {
	core::LogContext& log;
	core::AnimationSender& animation_sender;
	QuickslotSender& quickslot_sender;
	StatsSender& stats_sender;
	PerkSender& perk_sender;
	FeedbackSender& feedback_sender;
	StatsManager const& stats;

	Context(core::LogContext& log, core::AnimationSender& animation_sender,
		QuickslotSender& quickslot_sender, StatsSender& stats_sender,
		PerkSender& perk_sender, FeedbackSender& feedback_sender,
		StatsManager const& stats);
};

/// Sets the perk's level
/**
 *	This will set the object's perk's level to the given value. If the object
 *	did not have this perk yet, it will be added. If the level is set to zero,
 *	the perk is removed. If a perk is removed, a QuickslotEvent is propagated
 *	to remove the perk from any quickslot shortcut of this object.
 *
 *	@param context Perk context to use
 *	@param actor PerkData of the object
 *	@param perk PerkTemplate specifying the perk
 *	@param level target perk level
 */
void setPerkLevel(Context& context, PerkData& actor, PerkTemplate const& perk,
	std::size_t level);

/// Use the given perk
/**
 *	This will use the object's perk. It's mana costs are calculated and
 *	applied, if the actor has enough mana. If not, the function will return
 *	false. If there is enough mana, a stats event will be propagated in order
 *	to let the StatsSystem decrease the mana. Also an animation event is
 *	propagated to trigger the object's cast animation.
 *
 *	@pre The actor already has the given perk
 *	@param context Perk context to use
 *	@param actor PerkData of the object
 *	@param perk PerkTemplate specifying the perk
 *	@return true if perk use was triggered
 */
bool usePerk(Context& context, PerkData const& actor, PerkTemplate const& perk);

/// Calculate a perk's mana costs
/**
 *	@pre The actor already has the given perk
 *	@param actor PerkData of the object
 *	@param perk PerkTemplate specifying the perk
 *	@return amount of mana that is necessary to use the perk once
 */
unsigned int getManaCosts(PerkData const& actor, PerkTemplate const& perk);

/// Handles perk usage
/**
 *	This will trigger `usePerk()` and forward the perk event if succesful.
 *
 *	@pre The actor already has the given perk
 *	@param context Perk context to use
 *	@param actor PerkData of the object
 *	@param event PerkEvent that specifies the perk
 */
void onUse(Context& context, PerkData const& actor, PerkEvent const& event);

/// Handles perk improvement
/**
 *	This will increase the perk's level. If the perk did not exist yet, it
 *	will be added with level 1.
 *
 *	@param context Perk context to use
 *	@param actor PerkData of the object
 *	@param perk PerkTemplate specifying the perk
 */
void onIncrease(Context& context, PerkData& actor, PerkTemplate const& perk);

}  // ::perk_impl

// ---------------------------------------------------------------------------
// Public Perk API

/// Checks whether the actor as a specific perk
/**
 *	An object has a perk if the perk's level is greater than zero. Otherwise
 *	it does not have the perk.
 *
 *	@param actor PerkData of the object
 *	@param perk PerkTemplate specifying the perk
 *	@return true if the object has the perk
 */
bool hasPerk(PerkData const& actor, PerkTemplate const& perk);

/// Returns the perk's level
/**
 *	This will return zero if the object does not have the given perk.
 *
 *	@param actor PerkData of the object
 *	@param perk PerkTemplate specifying the perk
 *	@return perk level
 */
unsigned int getPerkLevel(PerkData const& actor, PerkTemplate const& perk);

// ---------------------------------------------------------------------------

/// The PerkSystem manages perk levels and usage
/**
 *	PerkEvents are handled for using a perk or changing a perk's level. If a
 *	perk is used, mana is consumed (via StatsEvent to e.g. the StatsSystem)
 *	and the PerkEvent is forwarded for actual perk usage. If a perk's level
 *	should be changed, it will be adjusted. If the perk was removed, a
 *	QuickslotEvent is propagated to the release it from shortcuts that may
 *	bound is in the first place.
 *	TrainingEvents are used to increase perk levels by 1. They are always sent
 *	by the PlayerSystem after decreasing the player's perk points.
 */
class PerkSystem
	// Event API
	: public utils::EventListener<PerkEvent, TrainingEvent>,
	  public utils::EventSender<core::AnimationEvent, QuickslotEvent,
		  StatsEvent, PerkEvent, FeedbackEvent>
	  // Component API
	  ,
	  public PerkManager {

  private:
	perk_impl::Context context;

  public:
	PerkSystem(core::LogContext& log, std::size_t max_objects, StatsManager const& stats);

	void handle(PerkEvent const& event);
	void handle(TrainingEvent const& event);

	void update(sf::Time const& elapsed);
};

}  // ::game

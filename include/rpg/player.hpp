#pragma once
#include <rpg/entity.hpp>
#include <rpg/event.hpp>

namespace rpg {

namespace player_impl {

/// Player context
struct Context {
	core::LogContext& log;

	ExpSender& exp_sender;
	TrainingSender& training_sender;
	FeedbackSender& feedback_sender;

	PlayerManager& player;
	StatsManager const& stats;

	Context(core::LogContext& log, ExpSender& exp_sender,
		TrainingSender& training_sender, FeedbackSender& feedback_sender,
		PlayerManager& player, StatsManager const& stats);
};

// ---------------------------------------------------------------------------

/// Gains experience to the given player
/**
 *	This will gain experience to the given player. Some stacked experience
 *	will be unstacked and applied as well. If a levelup occured, it will be
 *	handled. The actually gained experience (including unstacked experience)
 *	and whether a levelup occured or not, is forwarded here.
 *
 *	@param context PlayerContext to use
 *	@param data PlayerData to gain to
 *	@param exp Experience to gain
 */
void gainExp(Context& context, PlayerData& data, std::uint64_t exp);

/// Stacks experience to other players
/**
 *	This will stack the given experience to the given player if he is still
 *	alive. No levelup will happen when experience is stacked. The active
 *	player's stats are passed to respect the levels of the active player and
 *	the passive one. If their distance is too much, the experience will be
 *	reduced.
 *
 *	@param context PlayerContext to use
 *	@param actor StatsData of the active player
 *	@param target PlayerData to stack to
 *	@param exp Experience to stack
 */
void stackExp(Context& context, StatsData const& actor, PlayerData& target,
	std::uint64_t exp);

/// Handle experience gain
/**
 *	If the actor is dead, the function will immediately return. If he is still
 *	alive, he will gain experience and might levelup. All other players will
 *	stack experience (possibly with a small penality), which will be unstacked
 *	when they gain experience.
 *
 *	@param context PlayerContext to use
 *	@param data PlayerData to update
 *	@param exp Experience to handle
 */
void onExp(Context& context, PlayerData& data, std::uint64_t exp);

/// Handle a training event
/**
 *	If a perk should be trained, a perk point will be removed and the event
 *	is forwarded (to the PerkSystem, which actually increases the perk's
 *	level). If an attribute should be trained, an attribute point will be
 *	removed and the event is forwarded (to the StatsSystem, which actually
 *	increases the attribute's value). If not enough points are available, the
 *	event will be ignored at all.
 *
 *	@param context PlayerContext to use
 *	@param data PlayerData to train
 *	@param event TrainingEvent to trigger
 */
void onTraining(Context& context, PlayerData& data, TrainingEvent const& event);

}  // ::player_impl

// ---------------------------------------------------------------------------

/// The PlayerSystem handle's all player's experience and training
/**
 *	ExpEvents are handled by applying the given exp to the player and stacking
 *	if to his allies. If a player is dead, he will neither gain nor stack exp.
 *	Those ExpEvents are finally forwarded to notify other systems (e.g. Hud).
 *	TrainingEvents are used to trigger improvement of a perk's level or an
 *	attribute's value. Corresponding points are necessary, which can be
 *	collected by levelups. Those TrainingEvents are forwarded if enough points
 *	were available. So the other systems can apply the training after the
 *	PlayerSystem allowed it.
 */
class PlayerSystem
	// Event API
	: public utils::EventListener<ExpEvent, TrainingEvent>,
	  public utils::EventSender<ExpEvent, TrainingEvent, FeedbackEvent>
	  // Component API
	  ,
	  public PlayerManager {

  private:
	player_impl::Context context;

  public:
	PlayerSystem(core::LogContext& log, std::size_t max_objects, StatsManager const& stats);

	void handle(ExpEvent const& event);
	void handle(TrainingEvent const& event);

	void update(sf::Time const& elapsed);
};

}  // ::game

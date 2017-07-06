#include <utils/algorithm.hpp>
#include <rpg/balance.hpp>
#include <rpg/player.hpp>

namespace rpg {

namespace player_impl {

Context::Context(core::LogContext& log, ExpSender& exp_sender,
	TrainingSender& training_sender, FeedbackSender& feedback_sender,
	PlayerManager& player, StatsManager const& stats)
	: log{log}
	, exp_sender{exp_sender}
	, training_sender{training_sender}
	, feedback_sender{feedback_sender}
	, player{player}
	, stats{stats} {}

// ---------------------------------------------------------------------------

void gainExp(Context& context, PlayerData& data, std::uint64_t exp) {
	// unstack exp
	auto unstacked = std::min(data.stacked_exp, exp);
	data.stacked_exp -= unstacked;
	exp += unstacked;

	// increase exp
	data.exp += exp;

	ExpEvent event;
	event.actor = data.id;
	event.exp = exp;

	// determine number of levelups
	auto const lvl = context.stats.query(data.id).level;
	while (data.exp >= data.next_exp) {
		++event.levelup;
		data.next_exp = getNextExp(lvl + event.levelup + 1u);
	}
	if (event.levelup > 0u) {
		data.base_exp = getNextExp(lvl + event.levelup);
		data.attrib_points += event.levelup * ATTRIB_POINTS_PER_LEVEL;
		data.perk_points += event.levelup * PERK_POINTS_PER_LEVEL;
	}
	
	// propagate exp event
	context.exp_sender.send(event);
}

void stackExp(Context& context, StatsData const& actor, PlayerData& target,
	std::uint64_t exp) {
	auto const& stats = context.stats.query(target.id);
	if (stats.stats[Stat::Life] == 0u) {
		// not alive
		return;
	}

	target.stacked_exp += getStackedExp(actor.level, stats.level, exp);
}

void onExp(Context& context, PlayerData& data, std::uint64_t exp) {
	auto const& stats = context.stats.query(data.id);
	if (stats.stats[Stat::Life] == 0u) {
		// actor is not alive
		return;
	}

	// gain exp to active player
	gainExp(context, data, exp);

	// stack exp to other players
	for (auto& other : context.player) {
		if (other.id == data.id) {
			// do not stack to actor
			continue;
		}
		stackExp(context, stats, other, exp);
	}
}

void onTraining(
	Context& context, PlayerData& data, TrainingEvent const& event) {
	switch (event.type) {
		case TrainingEvent::Perk:
			ASSERT(event.perk != nullptr);
			if (data.perk_points > 0u) {
				--data.perk_points;
				context.training_sender.send(event);
			} else {
				FeedbackEvent ev;
				ev.actor = data.id;
				ev.type = FeedbackType::NotEnoughPerkPoints;
				context.feedback_sender.send(ev);
			}
			break;

		case TrainingEvent::Attrib:
			if (data.attrib_points > 0u) {
				--data.attrib_points;
				context.training_sender.send(event);
			} else {
				FeedbackEvent ev;
				ev.actor = data.id;
				ev.type = FeedbackType::NotEnoughAttribPoints;
				context.feedback_sender.send(ev);
			}
			break;
	}
}

}  // ::player_impl

// ---------------------------------------------------------------------------

PlayerSystem::PlayerSystem(core::LogContext& log, std::size_t max_objects, StatsManager const& stats)
	: utils::EventListener<ExpEvent, TrainingEvent>{}
	, utils::EventSender<ExpEvent, TrainingEvent, FeedbackEvent>{}
	, PlayerManager{max_objects}
	, context{log, *this, *this, *this, *this, stats} {}

void PlayerSystem::handle(ExpEvent const& event) {
	if (!has(event.actor)) {
		// no such component
		return;
	}
	auto& data = query(event.actor);

	player_impl::onExp(context, data, event.exp);
}

void PlayerSystem::handle(TrainingEvent const& event) {
	if (!has(event.actor)) {
		// no such component
		return;
	}
	auto& data = query(event.actor);

	player_impl::onTraining(context, data, event);
}

void PlayerSystem::update(sf::Time const& elapsed) {
	dispatch<ExpEvent>(*this);
	dispatch<TrainingEvent>(*this);

	propagate<ExpEvent>();
	propagate<TrainingEvent>();
	propagate<FeedbackEvent>();
}

}  // ::game

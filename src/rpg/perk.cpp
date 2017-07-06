#include <rpg/algorithm.hpp>
#include <rpg/balance.hpp>
#include <rpg/perk.hpp>

namespace rpg {

namespace perk_impl {

Context::Context(core::LogContext& log, core::AnimationSender& animation_sender,
	QuickslotSender& quickslot_sender, StatsSender& stats_sender,
	PerkSender& perk_sender, FeedbackSender& feedback_sender,
	StatsManager const& stats)
	: log{log}
	, animation_sender{animation_sender}
	, quickslot_sender{quickslot_sender}
	, stats_sender{stats_sender}
	, perk_sender{perk_sender}
	, feedback_sender{feedback_sender}
	, stats{stats} {}

void setPerkLevel(Context& context, PerkData& actor, PerkTemplate const& perk,
	std::size_t level) {
	// search existing perk
	auto i = utils::find_if(
		actor.perks, [&](Perk const& node) { return node.perk == &perk; });
	if (level > 0u) {
		// give perk
		if (i == actor.perks.end()) {
			// add new node
			actor.perks.emplace_back(perk, level);
		} else {
			// set level
			i->level = level;
		}
	} else {
		// remove perk
		if (i != actor.perks.end()) {
			// drop perk
			utils::pop(actor.perks, i);
			// trigger release from quickslots
			QuickslotEvent event;
			event.actor = actor.id;
			event.perk = &perk;
			event.type = QuickslotEvent::Release;
			context.quickslot_sender.send(event);
		}
	}
}

bool usePerk(
	Context& context, PerkData const& actor, PerkTemplate const& perk) {
	ASSERT(context.stats.has(actor.id));

	// check mana
	auto mana = getManaCosts(actor, perk);
	auto const& stats = context.stats.query(actor.id);
	if (stats.stats[Stat::Mana] < mana) {
		context.log.debug << "[Rpg/Perk] " << "Not enough mana\n";
		// trigger idle animation
		core::AnimationEvent event;
		event.actor = actor.id;
		event.type = core::AnimationEvent::Action;
		event.action = core::AnimationAction::Idle;
		context.animation_sender.send(event);

		// send feedback
		FeedbackEvent ev;
		ev.actor = actor.id;
		ev.type = FeedbackType::NotEnoughMana;
		context.feedback_sender.send(ev);

		return false;
	}

	// trigger mana consume
	StatsEvent stats_event;
	stats_event.actor = actor.id;
	stats_event.delta[Stat::Mana] = -mana;
	context.stats_sender.send(stats_event);

	return true;
}

unsigned int getManaCosts(PerkData const& actor, PerkTemplate const& perk) {
	// query perk level
	auto level = getPerkLevel(actor, perk);
	ASSERT(level > 0u);  // means that the actor has the perk
	float factor{0.f};
	for (auto const & pair: perk.damage) {
		factor += pair.second;
	}
	return getPerkCosts(factor, level);
}

void onUse(Context& context, PerkData const& actor, PerkEvent const& event) {
	if (perk_impl::usePerk(context, actor, *event.perk)) {
		// success! forward it
		context.perk_sender.send(event);
	}
}

void onIncrease(Context& context, PerkData& actor, PerkTemplate const& perk) {
	auto level = getPerkLevel(actor, perk);
	setPerkLevel(context, actor, perk, level + 1u);
}

}  // ::perk_impl

// ---------------------------------------------------------------------------

bool hasPerk(PerkData const& actor, PerkTemplate const& perk) {
	// search existing perk
	auto i = utils::find_if(
		actor.perks, [&](Perk const& node) { return node.perk == &perk; });
	return (i != actor.perks.end()) && (i->level > 0u);
}

unsigned int getPerkLevel(PerkData const& actor, PerkTemplate const& perk) {
	// search existing perk
	auto i = utils::find_if(
		actor.perks, [&](Perk const& node) { return node.perk == &perk; });
	if (i != actor.perks.end()) {
		return i->level;
	}
	return 0u;
}

// ---------------------------------------------------------------------------

PerkSystem::PerkSystem(core::LogContext& log, std::size_t max_objects, StatsManager const& stats)
	: utils::EventListener<PerkEvent, TrainingEvent>{}
	, utils::EventSender<core::AnimationEvent, QuickslotEvent, StatsEvent,
		  PerkEvent, FeedbackEvent>{}
	, PerkManager{max_objects}
	, context{log, *this, *this, *this, *this, *this, stats} {}

void PerkSystem::handle(PerkEvent const& event) {
	if (!has(event.actor)) {
		// no such component
		return;
	}
	ASSERT(event.perk != nullptr);
	auto& actor = query(event.actor);

	switch (event.type) {
		case PerkEvent::Set:
			perk_impl::setPerkLevel(context, actor, *event.perk, event.level);
			break;

		case PerkEvent::Use:
			perk_impl::onUse(context, actor, event);
			break;
	}
}

void PerkSystem::handle(TrainingEvent const& event) {
	if (!has(event.actor)) {
		// no such component
		return;
	}
	auto& actor = query(event.actor);

	if (event.type == TrainingEvent::Perk) {
		ASSERT(event.perk != nullptr);
		perk_impl::onIncrease(context, actor, *event.perk);
	}
}

void PerkSystem::update(sf::Time const& elapsed) {
	dispatch<TrainingEvent>(*this);
	dispatch<PerkEvent>(*this);

	propagate<core::AnimationEvent>();
	propagate<QuickslotEvent>();
	propagate<StatsEvent>();
	propagate<PerkEvent>();
	propagate<FeedbackEvent>();
}

}  // ::game

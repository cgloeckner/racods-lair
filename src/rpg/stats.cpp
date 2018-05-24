#include <rpg/algorithm.hpp>
#include <rpg/balance.hpp>
#include <rpg/stats.hpp>

namespace rpg {

namespace stats_impl {

Context::Context(
	core::LogContext& log, StatsSender& stats_sender, DeathSender& death_sender)
	: log{log}, stats_sender{stats_sender}, death_sender{death_sender} {}

void cap(unsigned int& value, unsigned int max) {
	if (value > max) {
		value = max;
	}
}

bool applyStats(Context& context, StatsData& actor, StatsEvent& event) {
	if (actor.stats[Stat::Life] == 0u || actor.godmode) {
		// cannot apply - reset delta
		event.delta = decltype(event.delta){};
		return false;
	}
	
	// determine maximum stats
	utils::EnumMap<Stat, int> max;
	max[Stat::Life] = actor.properties[Property::MaxLife];
	max[Stat::Mana] = actor.properties[Property::MaxMana];
	max[Stat::Stamina] = actor.properties[Property::MaxStamina];

	// apply changes
	bool has_changed = false;
	for (auto& pair : actor.stats) {
		// check bounds
		auto& value = event.delta[pair.first];
		if (value > 0) {
			// avoid overflow if already fully healed
			if (pair.second == max[pair.first]) {
				event.delta[pair.first] = 0;
				continue;
			}
			auto delta = max[pair.first] - pair.second;
			if (value > delta) {
				// prevent overflow
				value = delta;
			}
			auto _max = max[pair.first];
			ASSERT(pair.second + value <= _max);
		} else if (value < 0) {
			// avoid underflow if already dead
			if (pair.second == 0) {
				event.delta[pair.first] = 0;
				continue;
			}
			if (std::abs(value) > pair.second) {
				// prevent underflow
				value = -pair.second;
			}
			ASSERT(static_cast<int>(pair.second) + value >= 0);
		}
		// apply value
		if (value != 0) {
			pair.second += value;
			ASSERT(pair.second <= max[pair.first]);
			has_changed = true;
		}
	}
	
	// check whether died or nor
	if (actor.stats[Stat::Life] == 0u) {
		// propagate death
		DeathEvent death_event;
		death_event.actor = actor.id;
		death_event.causer = event.causer;
		context.death_sender.send(death_event);
	}

	return has_changed;
}

// ---------------------------------------------------------------------------

void applyFactor(std::uint32_t& value, float factor) {
	value = static_cast<std::uint32_t>(std::ceil(value * factor));
}

void refresh(StatsData& actor) {
	// refresh base properties
	actor.base_props[Property::MaxLife] = getMaxLife(actor.attributes, actor.level);
	actor.base_props[Property::MaxMana] = getMaxMana(actor.attributes, actor.level);
	actor.base_props[Property::MaxStamina] = getMaxStamina(actor.attributes, actor.level);
	actor.base_props[Property::MeleeBase] = getMeleeBase(actor.attributes, actor.level);
	actor.base_props[Property::RangeBase] = getRangeBase(actor.attributes, actor.level);
	actor.base_props[Property::MagicBase] = getMagicBase(actor.attributes, actor.level);

	// refresh total properties
	actor.properties = actor.base_props;
	actor.properties += actor.prop_boni;
	
	// apply factor to properties
	applyFactor(actor.properties[Property::MaxLife], actor.factor);
	applyFactor(actor.properties[Property::MaxMana], actor.factor);
	applyFactor(actor.properties[Property::MaxStamina], actor.factor);
}

void addBoni(Context& context, StatsData& actor, StatsBoni const& boni) {
	actor.base_def += boni.defense;
	actor.prop_boni += boni.properties;
	refresh(actor);
}

void removeBoni(Context& context, StatsData& actor, StatsBoni const& boni) {
	actor.base_def -= boni.defense;
	actor.prop_boni -= boni.properties;
	refresh(actor);
}

void increaseAttribute(Context& context, StatsData& actor, Attribute attrib) {
	// determine delta between stats and max stats
	utils::EnumMap<Stat, std::uint32_t> delta{0u};
	delta[Stat::Life] = actor.properties[Property::MaxLife] - actor.stats[Stat::Life];
	delta[Stat::Mana] = actor.properties[Property::MaxMana] - actor.stats[Stat::Mana];
	delta[Stat::Stamina] = actor.properties[Property::MaxStamina] - actor.stats[Stat::Stamina];
	
	++actor.attributes[attrib];
	refresh(actor);
	
	// apply previous delta
	actor.stats[Stat::Life] = actor.properties[Property::MaxLife] - delta[Stat::Life];
	actor.stats[Stat::Mana] = actor.properties[Property::MaxMana] - delta[Stat::Mana];
	actor.stats[Stat::Stamina] = actor.properties[Property::MaxStamina] - delta[Stat::Stamina];
}

void onLevelup(Context& context, StatsData& actor, unsigned int delta) {
	actor.level += delta;
	refresh(actor);
	
	// restore stats
	actor.stats[Stat::Life] = actor.properties[Property::MaxLife];
	actor.stats[Stat::Mana] = actor.properties[Property::MaxMana];
	actor.stats[Stat::Stamina] = actor.properties[Property::MaxStamina];
}

}  // ::stats_impl

// ---------------------------------------------------------------------------

StatsSystem::StatsSystem(core::LogContext& log, std::size_t max_objects)
	: utils::EventListener<StatsEvent, BoniEvent, TrainingEvent, ExpEvent>{}
	, utils::EventSender<StatsEvent, DeathEvent>{}
	, StatsManager{max_objects}
	, context{log, *this, *this} {}

void StatsSystem::handle(StatsEvent const& event) {
	if (!has(event.actor)) {
		// no such component
		return;
	}

	auto& actor = query(event.actor);
	auto copy = event;

	if (stats_impl::applyStats(context, actor, copy)) {
		// forward actual stats change
		send(copy);
	}
}

void StatsSystem::handle(BoniEvent const& event) {
	if (!has(event.actor)) {
		// no such component
		return;
	}
	auto& actor = query(event.actor);
	ASSERT(event.boni != nullptr);

	switch (event.type) {
		case BoniEvent::Add:
			stats_impl::addBoni(context, actor, *event.boni);
			break;

		case BoniEvent::Remove:
			stats_impl::removeBoni(context, actor, *event.boni);
			break;
	}
}

void StatsSystem::handle(ExpEvent const& event) {
	if (!has(event.actor)) {
		// no such component
		return;
	}
	auto& actor = query(event.actor);

	if (event.levelup > 0u) {
		stats_impl::onLevelup(context, actor, event.levelup);
	}
}

void StatsSystem::handle(TrainingEvent const& event) {
	if (!has(event.actor)) {
		// no such component
		return;
	}
	auto& actor = query(event.actor);

	if (event.type == TrainingEvent::Attrib) {
		stats_impl::increaseAttribute(context, actor, event.attrib);
	}
}

void StatsSystem::update(sf::Time const& elapsed) {
	dispatch<StatsEvent>(*this);
	dispatch<BoniEvent>(*this);
	dispatch<TrainingEvent>(*this);
	dispatch<ExpEvent>(*this);

	propagate<DeathEvent>();
	propagate<StatsEvent>();
}

}  // ::game

#include <Thor/Math.hpp>

#include <utils/math2d.hpp>
#include <rpg/algorithm.hpp>
#include <rpg/balance.hpp>
#include <rpg/combat.hpp>
#include <rpg/perk.hpp>

namespace rpg {

namespace combat_impl {

float const MAX_MELEE_DISTANCE = 2.f;

Context::Context(core::LogContext& log, StatsSender& stats_sender,
	ExpSender& exp_sender, EffectSender& effect_sender,
	ProjectileSender& projectile_sender, SpawnSender& respawn_sender,
	core::MovementManager const& movement, ProjectileManager const& projectile,
	PerkManager const& perk, StatsManager const& stats,
	InteractManager const& interact, float variance)
	: log{log}
	, stats_sender{stats_sender}
	, exp_sender{exp_sender}
	, effect_sender{effect_sender}
	, projectile_sender{projectile_sender}
	, respawn_sender{respawn_sender}
	, movement{movement}
	, projectile{projectile}
	, perk{perk}
	, stats{stats}
	, interact{interact}
	, variance{variance}
	, projectiles{} {}

// ---------------------------------------------------------------------------

StatsData const* getAttacker(Context const& context, CombatEvent const& event) {
	StatsData const* actor = nullptr;
	if (event.actor == 0u) {
		// there is no attacker (when an effect is applied)
		return actor;
	}

	auto id = event.actor;
	if (context.projectile.has(id)) {
		// actor is a bullet
		auto const& projectile = context.projectile.query(id);
		if (projectile.owner == 0u) {
			// there is no projectile's owner (like a trap's bullet explodes)
			return actor;
		}
		// use projectile's owner as actor
		id = projectile.owner;
	}

	if (context.stats.has(id)) {
		actor = &context.stats.query(id);
	}

	return actor;
}

DamageMap getWeaponDamage(StatsData const& actor, ItemTemplate const& weapon) {
	// determine weapon damage
	DamageMap damage{0u};
	unsigned int bonus{0u};
	if (weapon.melee) {
		bonus = actor.properties[Property::MeleeBase];
	} else {
		bonus = actor.properties[Property::RangeBase];
	}
	for (auto& pair: weapon.damage) {
		if (pair.second <= 0.f) {
			continue;
		}
		damage[pair.first] = getDamageBonus(pair.second * bonus, actor.level);
	}
	return damage;
}

DamageMap getWeaponDamage(StatsData const& actor, ItemTemplate const* primary,
	ItemTemplate const* secondary) {
	DamageMap damage{0u};
	if (primary != nullptr) {
		// use weapons' damage
		damage += getWeaponDamage(actor, *primary);
		if (secondary != nullptr) {
			damage += getWeaponDamage(actor, *secondary);
		}
	} else {
		ASSERT(secondary == nullptr);
		// use fists
		damage[DamageType::Blunt] = getDamageBonus(actor.properties[Property::MeleeBase], actor.level);
	}
	return damage;
}

DamageMap getPerkDamage(PerkData const& perk_data, StatsData const& actor,
	PerkTemplate const& perk) {
	// determine perk damage
	DamageMap damage{0u};
	auto level = getPerkLevel(perk_data, perk);
	auto bonus = actor.properties[Property::MagicBase];
	for (auto& pair: perk.damage) {
		if (pair.second <= 0.f) {
			continue;
		}
		damage[pair.first] = getPerkBonus(pair.second, bonus, level);
	}
	return damage;
}

DamageMap getEffectDamage(StatsData const & actor, EffectTemplate const & effect) {
	DamageMap damage;
	for (auto& pair: damage) {
		pair.second = getEffectValue(effect.damage[pair.first], actor.level);
	}
	return damage;
}

StatsMap getPerkRecovery(PerkData const& perk_data, StatsData const& actor,
	PerkTemplate const& perk) {
	StatsMap stats{0u};
	auto bonus = actor.properties[Property::MagicBase];
	auto level = getPerkLevel(perk_data, perk);
	std::uint32_t sum{0u};
	for (auto& pair: perk.recover) {
		if (pair.second > 0u) {
			stats[pair.first] = getPerkBonus(pair.second, bonus, level);
		} else {
			stats[pair.first] = -getPerkBonus(std::abs(pair.second), bonus, level);
		}
		sum += std::abs(stats[pair.first]);
	}
	return stats;
}

StatsMap getEffectRecovery(StatsData const & actor, EffectTemplate const & effect) {
	StatsMap stats;
	stats[rpg::Stat::Life] = std::ceil(effect.recover[rpg::Stat::Life] * actor.properties[rpg::Property::MaxLife]);
	stats[rpg::Stat::Mana] = std::ceil(effect.recover[rpg::Stat::Mana] * actor.properties[rpg::Property::MaxMana]);
	stats[rpg::Stat::Stamina] = std::ceil(effect.recover[rpg::Stat::Stamina] * actor.properties[rpg::Property::MaxStamina]);
	return stats;
}

DamageMap getDamage(Context const& context, CombatMetaData const& data,
	StatsData const* actor, StatsData const & target) {
	switch (data.emitter) {
		case EmitterType::Weapon:
			ASSERT(actor != nullptr);
			return getWeaponDamage(*actor, data.primary, data.secondary);

		case EmitterType::Perk:
			ASSERT(actor != nullptr);
			ASSERT(data.perk != nullptr);
			ASSERT(context.perk.has(actor->id));
			return getPerkDamage(
				context.perk.query(actor->id), *actor, *data.perk);

		case EmitterType::Effect:
			ASSERT(data.effect != nullptr);
			return getEffectDamage(target, *data.effect);

		case EmitterType::Trap:
			ASSERT(data.trap != nullptr);
			return data.trap->damage;
	}
}

DamageMap getDefense(StatsData const & target) {
	DamageMap defense;
	auto bonus = (target.properties[Property::MeleeBase]
		+ target.properties[Property::RangeBase]
		+ target.properties[Property::MagicBase]) / 3.f;
	//bonus = std::floor(bonus * / 2.f); // hardcoded fix
	for (auto const & pair: target.base_def) {
		defense[pair.first] = getDefenseBonus(pair.second * bonus, target.level);
	}
	return defense;
}

StatsMap getRecovery(Context const& context, CombatMetaData const& data,
	StatsData const* actor, StatsData const & target) {
	switch (data.emitter) {
		case EmitterType::Perk:
			ASSERT(actor != nullptr);
			ASSERT(data.perk != nullptr);
			ASSERT(context.perk.has(actor->id));
			return getPerkRecovery(
				context.perk.query(actor->id), *actor, *data.perk);

		case EmitterType::Effect:
			ASSERT(data.effect != nullptr);
			return getEffectRecovery(target, *data.effect);

		default:
			// other cases do not recover
			// tba: implement leech will change this!
			return StatsMap{0};
	}
}

std::vector<EffectEmitter> getEffectEmitters(CombatMetaData const& data) {
	std::vector<EffectEmitter> emitters;

	switch (data.emitter) {
		case EmitterType::Weapon:
			if (data.primary != nullptr) {
				emitters.push_back(data.primary->effect);
			}
			if (data.secondary != nullptr) {
				emitters.push_back(data.secondary->effect);
			}
			break;

		case EmitterType::Perk:
			ASSERT(data.perk != nullptr);
			emitters.push_back(data.perk->effect);
			break;

		case EmitterType::Trap:
			ASSERT(data.trap != nullptr);
			emitters.push_back(data.trap->effect);
			break;

		default:
			// other cases do not inflict effects
			break;
	}

	return emitters;
}

void randomize(Context const& context, int& value) {
	// save sign
	bool negative = value < 0;
	value = std::abs(value);
	// determine [min, max]
	auto min = static_cast<int>(std::ceil(value * (1.f - context.variance)));
	auto max = static_cast<int>(value * (1.f + context.variance));
	// apply sign
	if (negative) {
		std::swap(min, max);
		min = -min;
		max = -max;
	}
	// pick random value
	ASSERT(min <= max);
	value = thor::random(min, max);
}

void onCombat(Context& context, CombatEvent const& event) {
	if (!context.stats.has(event.target)) {
		// note: the target could be a barrier
		bool is_barrier{false};
		if (context.projectile.has(event.actor) &&
			context.interact.has(event.target)) {
			is_barrier = context.interact.query(event.target).type ==
						 InteractType::Barrier;
		}
		if (is_barrier) {
			// mark projectile for deletion
			if (!utils::contains(context.projectiles, event.actor)) {
				// note: multiple combats can be triggered by one projectile
				context.projectiles.push_back(event.actor);
			}
		} else {
			// no such component
			context.log.debug << "[Rpg/Combat] " << "Combat against #" << event.target
							  << " was ignored - object not found\n";
		}
		return;
	}

	StatsEvent stats;
	stats.actor = event.target;
	ExpEvent exp;
	EffectEvent effect;
	effect.actor = event.target;
	effect.type = EffectEvent::Add;
	SpawnEvent respawn;
	respawn.actor = event.target;
	respawn.respawn = true;

	// determine participated objects
	auto actor = getAttacker(context, event);
	auto const& target = context.stats.query(event.target);

	bool revive =
		(event.meta_data.perk != nullptr && event.meta_data.perk->revive);
	if (actor != nullptr && event.actor == actor->id) {
		respawn.causer = actor->id;
		// check actor health
		if (actor->stats[Stat::Life] == 0u) {
			context.log.debug << "[Rpg/Combat] " << "Combat of #" << event.actor
							  << " was ignored - actor died\n";
			return;
		}
		// check target health
		bool dead = (target.stats[Stat::Life] == 0u);
		if (dead && !revive) {
			// cannot harm already dead target
			context.log.debug << "[Rpg/Combat] " << "Combat of #" << event.actor
							  << " was ignored - target died\n";
			return;
		} else if (!dead && revive) {
			// cannot revive already living target
			revive = false;
		}
		if (event.meta_data.primary == nullptr ||
			event.meta_data.primary->melee) {
			// check melee distance
			auto const& actor_move = context.movement.query(event.actor);
			auto const& target_move = context.movement.query(event.target);
			auto dist = utils::distance(actor_move.pos, target_move.pos);
			if (dist > MAX_MELEE_DISTANCE * MAX_MELEE_DISTANCE || actor_move.scene != target_move.scene) {
				// out of melee range
				context.log.debug << "[Rpg/Combat] " << "Combat between #" << event.actor
								  << " and #" << event.target
								  << " was ignored - out of melee range\n";
				return;
			}
		}
	} else {
		// check target health
		if (target.stats[Stat::Life] == 0u) {
			// cannot harm dead target
			context.log.debug << "[Rpg/Combat] " << "Combat of #" << event.actor
							  << " was ignored - target died\n";
			return;
		}
	}

	// determine damage and defense (including randomization)
	auto damage = getDamage(context, event.meta_data, actor, target);
	auto defense = getDefense(target);

	// calculate actual damage
	unsigned int actual_damage = 0u;
	for (auto const& pair : defense) {
		if (damage[pair.first] > pair.second) {
			actual_damage += (damage[pair.first] - pair.second);
		}
	}

	// determine total stats change (e.g. apply effect)
	stats.delta = getRecovery(context, event.meta_data, actor, target);
	stats.delta[Stat::Life] -= actual_damage;

	// randomize stats change
	for (auto& pair : stats.delta) {
		randomize(context, pair.second);
	}

	if (actor != nullptr) {
		// determine exp
		for (auto const& pair : stats.delta) {
			exp.exp += getExpGain(std::abs(pair.second), actor->level);
		}
		
		effect.causer = actor->id;
		stats.causer = actor->id;
		exp.actor = actor->id;
		
		// propagate exp
		context.exp_sender.send(exp);
	}

	// determine possible effects
	auto effects = getEffectEmitters(event.meta_data);
	for (auto const& emitter : effects) {
		if (emitter.effect == nullptr) {
			continue;
		}
		if (thor::random(0.f, 1.f) <= emitter.ratio) {
			// inflcit effect
			effect.effect = emitter.effect;
			context.effect_sender.send(effect);
		}
	}

	// propagate stats
	context.stats_sender.send(stats);

	// propagate respawn event
	if (revive) {
		context.respawn_sender.send(respawn);
	}

	if (event.actor > 0u && context.projectile.has(event.actor)) {
		// mark projectile for deletion
		if (!utils::contains(context.projectiles, event.actor)) {
			// note: multiple combats can be triggered by one projectile
			context.projectiles.push_back(event.actor);
		}
	}
}

void onUpdate(Context& context, sf::Time const& elapsed) {
	ProjectileEvent event;
	event.type = ProjectileEvent::Destroy;
	for (auto id : context.projectiles) {
		event.id = id;
		context.projectile_sender.send(event);
	}
	context.projectiles.clear();
}

}  // ::combat_impl

// ---------------------------------------------------------------------------

CombatSystem::CombatSystem(core::LogContext& log,
	core::MovementManager const& movement, ProjectileManager const& projectile,
	PerkManager const& perk, StatsManager const& stats,
	InteractManager const& interact, float variance)
	: utils::EventListener<CombatEvent>{}
	, utils::EventSender<StatsEvent, ExpEvent, EffectEvent, ProjectileEvent,
		  SpawnEvent>{}
	, context{log, *this, *this, *this, *this, *this, movement, projectile,
		  perk, stats, interact, variance} {}

void CombatSystem::handle(CombatEvent const& event) {
	combat_impl::onCombat(context, event);
}

void CombatSystem::update(sf::Time const& elapsed) {
	dispatch<CombatEvent>(*this);

	onUpdate(context, elapsed);

	propagate<StatsEvent>();
	propagate<ExpEvent>();
	propagate<EffectEvent>();
	propagate<ProjectileEvent>();
	propagate<SpawnEvent>();
}

void CombatSystem::clear() {
}

}  // ::game

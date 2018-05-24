#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>
#include <testsuite/singleton.hpp>

#include <rpg/balance.hpp>
#include <rpg/combat.hpp>

struct CombatFixture {
	core::LogContext log;
	core::IdManager ids;
	std::vector<core::ObjectID> objects;

	rpg::StatsSender stats_sender;
	rpg::ExpSender exp_sender;
	rpg::EffectSender effect_sender;
	rpg::ProjectileSender projectile_sender;
	rpg::SpawnSender respawn_sender;

	core::MovementManager movement;
	rpg::ProjectileManager projectile;
	rpg::PerkManager perk;
	rpg::StatsManager stats;
	rpg::InteractManager interact;

	rpg::combat_impl::Context context;

	rpg::ItemTemplate weapon, weapon2;
	rpg::PerkTemplate revive, spell, spell2;
	rpg::EffectTemplate effect;
	rpg::TrapTemplate trap;

	CombatFixture()
		: log{}
		, ids{}
		, objects{}
		, stats_sender{}
		, exp_sender{}
		, effect_sender{}
		, projectile_sender{}
		, respawn_sender{}
		, projectile{}
		, perk{}
		, stats{}
		, interact{}
		, context{log, stats_sender, exp_sender, effect_sender,
			  projectile_sender, respawn_sender, movement, projectile, perk,
			  stats, interact, 0.f}
		, weapon{}
		, weapon2{}
		, revive{}
		, spell{}
		, spell2{}
		, effect{}
		, trap{} {
		weapon.melee = true;
		weapon.two_handed = false;
		weapon.damage[rpg::DamageType::Blade] = 0.7f;
		weapon.damage[rpg::DamageType::Poison] = 0.3f;
		weapon.effect.effect = &effect;
		weapon.effect.ratio = 1.f;
		weapon2 = weapon;
		weapon2.melee = false;
		weapon2.two_handed = true;
		weapon2.effect.effect = &effect;
		weapon2.effect.ratio = 1.f;
		revive.revive = true;
		revive.recover[rpg::Stat::Life] = 20;
		spell.damage[rpg::DamageType::Magic] = 0.1f;
		spell.damage[rpg::DamageType::Ice] = 0.9f;
		spell.effect.effect = &effect;
		spell.effect.ratio = 1.f;
		spell2.recover[rpg::Stat::Life] = 2.f;
		spell2.recover[rpg::Stat::Stamina] = 0.5f;
		spell2.effect.effect = &effect;
		spell2.effect.ratio = 1.f;
		trap.damage[rpg::DamageType::Blunt] = 150;
		trap.damage[rpg::DamageType::Bullet] = 150;
		trap.effect.effect = &effect;
		trap.effect.ratio = 1.f;
	}

	rpg::StatsData& addAvatar() {
		auto id = ids.acquire();
		objects.push_back(id);
		movement.acquire(id);
		auto& p = perk.acquire(id);
		p.perks.emplace_back(spell, 10u);
		p.perks.emplace_back(spell2, 10u);
		auto& s = stats.acquire(id);
		for (auto& pair : s.stats) {
			pair.second = 500;
		}
		s.level = 10;
		s.properties[rpg::Property::MaxLife] = 500;
		s.properties[rpg::Property::MaxMana] = 500;
		s.properties[rpg::Property::MaxStamina] = 500;
		s.properties[rpg::Property::MeleeBase] = 100;
		s.properties[rpg::Property::RangeBase] = 50;
		s.properties[rpg::Property::MagicBase] = 70;
		for (auto& pair : s.base_def) {
			pair.second = 1.f;
		}
		return s;
	}

	core::ObjectID addBullet(
		core::ObjectID owner, rpg::CombatMetaData const& data) {
		auto id = ids.acquire();
		objects.push_back(id);
		auto& proj = projectile.acquire(id);
		proj.owner = owner;
		proj.meta_data = data;
		return id;
	}

	rpg::InteractData& addInteractable() {
		auto id = ids.acquire();
		objects.push_back(id);
		return interact.acquire(id);
	}

	rpg::CombatMetaData getMetaData() {
		rpg::CombatMetaData data;
		data.emitter = rpg::EmitterType::Weapon;
		data.primary = nullptr;
		data.secondary = nullptr;
		return data;
	}

	rpg::CombatMetaData getMetaData(rpg::ItemTemplate const& primary) {
		rpg::CombatMetaData data;
		data.emitter = rpg::EmitterType::Weapon;
		data.primary = &primary;
		data.secondary = nullptr;
		return data;
	}

	rpg::CombatMetaData getMetaData(
		rpg::ItemTemplate const& primary, rpg::ItemTemplate const& secondary) {
		rpg::CombatMetaData data;
		data.emitter = rpg::EmitterType::Weapon;
		data.primary = &primary;
		data.secondary = &secondary;
		return data;
	}

	rpg::CombatMetaData getMetaData(rpg::PerkTemplate const& perk) {
		rpg::CombatMetaData data;
		data.emitter = rpg::EmitterType::Perk;
		data.perk = &perk;
		return data;
	}

	rpg::CombatMetaData getMetaData(rpg::EffectTemplate const& effect) {
		rpg::CombatMetaData data;
		data.emitter = rpg::EmitterType::Effect;
		data.effect = &effect;
		return data;
	}

	rpg::CombatMetaData getMetaData(rpg::TrapTemplate const& trap) {
		rpg::CombatMetaData data;
		data.emitter = rpg::EmitterType::Trap;
		data.trap = &trap;
		return data;
	}

	void reset() {
		effect = rpg::EffectTemplate{};
		
		for (auto id : objects) {
			if (movement.has(id)) {
				movement.release(id);
			}
			if (projectile.has(id)) {
				projectile.release(id);
			}
			if (perk.has(id)) {
				perk.release(id);
			}
			if (stats.has(id)) {
				stats.release(id);
			}
			if (interact.has(id)) {
				interact.release(id);
			}
		}
		objects.clear();
		ids.reset();
		movement.cleanup();
		projectile.cleanup();
		perk.cleanup();
		stats.cleanup();
		interact.cleanup();

		context.projectiles.clear();
		stats_sender.clear();
		exp_sender.clear();
		effect_sender.clear();
		projectile_sender.clear();
		respawn_sender.clear();
		
		// clear logs
		log.debug.clear();
		log.warning.clear();
		log.error.clear();
	}
};

BOOST_AUTO_TEST_SUITE(combat_test)

BOOST_AUTO_TEST_CASE(getAttacker_return_stats_of_actor_using_fists) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto& actor = fix.addAvatar();
	// trigger avatar attack
	rpg::CombatEvent event;
	event.actor = actor.id;
	event.meta_data = fix.getMetaData();
	auto attacker = rpg::combat_impl::getAttacker(fix.context, event);

	BOOST_CHECK_EQUAL(attacker, &actor);
}

BOOST_AUTO_TEST_CASE(getAttacker_return_stats_of_actor_using_one_weapon) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto& actor = fix.addAvatar();
	// trigger avatar attack
	rpg::CombatEvent event;
	event.actor = actor.id;
	event.meta_data = fix.getMetaData(fix.weapon);
	auto attacker = rpg::combat_impl::getAttacker(fix.context, event);

	BOOST_CHECK_EQUAL(attacker, &actor);
}

BOOST_AUTO_TEST_CASE(getAttacker_return_stats_of_actor_using_two_weapons) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto& actor = fix.addAvatar();
	// trigger avatar attack
	rpg::CombatEvent event;
	event.actor = actor.id;
	event.meta_data = fix.getMetaData(fix.weapon, fix.weapon);
	auto attacker = rpg::combat_impl::getAttacker(fix.context, event);

	BOOST_CHECK_EQUAL(attacker, &actor);
}

BOOST_AUTO_TEST_CASE(getAttacker_return_stats_of_actor_using_range_weapon) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto& actor = fix.addAvatar();
	auto meta = fix.getMetaData(fix.weapon);
	auto proj = fix.addBullet(actor.id, meta);
	// trigger bullet explosion
	rpg::CombatEvent event;
	event.actor = proj;
	event.meta_data = meta;
	auto attacker = rpg::combat_impl::getAttacker(fix.context, event);

	BOOST_CHECK_EQUAL(attacker, &actor);
}

BOOST_AUTO_TEST_CASE(getAttacker_return_stats_of_actor_casting) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto& actor = fix.addAvatar();
	// trigger avatar attack
	rpg::CombatEvent event;
	event.actor = actor.id;
	event.meta_data = fix.getMetaData(fix.spell);
	auto attacker = rpg::combat_impl::getAttacker(fix.context, event);

	BOOST_CHECK_EQUAL(attacker, &actor);
}

BOOST_AUTO_TEST_CASE(getAttacker_return_stats_of_bullet_from_spell) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto& actor = fix.addAvatar();
	auto meta = fix.getMetaData(fix.spell);
	auto proj = fix.addBullet(actor.id, meta);
	// trigger bullet explosion
	rpg::CombatEvent event;
	event.actor = proj;
	event.meta_data = meta;
	auto attacker = rpg::combat_impl::getAttacker(fix.context, event);

	BOOST_CHECK_EQUAL(attacker, &actor);
}

BOOST_AUTO_TEST_CASE(getAttacker_return_nullptr_for_effect) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	// trigger effect damage
	rpg::CombatEvent event;
	event.actor = 0u;
	event.meta_data = fix.getMetaData(fix.effect);
	auto attacker = rpg::combat_impl::getAttacker(fix.context, event);

	BOOST_CHECK(attacker == nullptr);
}

BOOST_AUTO_TEST_CASE(getAttacker_return_nullptr_for_trap) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	// trigger effect damage
	rpg::CombatEvent event;
	event.actor = 0u;
	event.meta_data = fix.getMetaData(fix.trap);
	auto attacker = rpg::combat_impl::getAttacker(fix.context, event);

	BOOST_CHECK(attacker == nullptr);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(getWeaponDamage_adds_melee_bonus_for_melee_weapon) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto& actor = fix.addAvatar();
	auto damage = rpg::combat_impl::getWeaponDamage(actor, fix.weapon);
	auto bonus = actor.properties[rpg::Property::MeleeBase];
	BOOST_CHECK_EQUAL(damage[rpg::DamageType::Blade], rpg::getDamageBonus(bonus * 0.7f, actor.level));
	BOOST_CHECK_EQUAL(damage[rpg::DamageType::Blunt], 0u);
	BOOST_CHECK_EQUAL(damage[rpg::DamageType::Bullet], 0u);
	BOOST_CHECK_EQUAL(damage[rpg::DamageType::Magic], 0u);
	BOOST_CHECK_EQUAL(damage[rpg::DamageType::Fire], 0u);
	BOOST_CHECK_EQUAL(damage[rpg::DamageType::Ice], 0u);
	BOOST_CHECK_EQUAL(damage[rpg::DamageType::Poison], rpg::getDamageBonus(bonus * 0.3f, actor.level));
}

BOOST_AUTO_TEST_CASE(getWeaponDamage_adds_range_bonus_for_range_weapon) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto& actor = fix.addAvatar();
	auto damage = rpg::combat_impl::getWeaponDamage(actor, fix.weapon2);
	auto bonus = actor.properties[rpg::Property::RangeBase];
	BOOST_CHECK_EQUAL(damage[rpg::DamageType::Blade], rpg::getDamageBonus(bonus * 0.7f, actor.level));
	BOOST_CHECK_EQUAL(damage[rpg::DamageType::Blunt], 0u);
	BOOST_CHECK_EQUAL(damage[rpg::DamageType::Bullet], 0u);
	BOOST_CHECK_EQUAL(damage[rpg::DamageType::Magic], 0u);
	BOOST_CHECK_EQUAL(damage[rpg::DamageType::Fire], 0u);
	BOOST_CHECK_EQUAL(damage[rpg::DamageType::Ice], 0u);
	BOOST_CHECK_EQUAL(damage[rpg::DamageType::Poison], rpg::getDamageBonus(bonus * 0.3f, actor.level));
}

BOOST_AUTO_TEST_CASE(getWeaponDamage_adds_melee_bonus_for_two_melee_weapon) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto& actor = fix.addAvatar();
	auto damage =
		rpg::combat_impl::getWeaponDamage(actor, &fix.weapon, &fix.weapon);
	auto bonus = actor.properties[rpg::Property::MeleeBase];
	BOOST_CHECK_EQUAL(damage[rpg::DamageType::Blade], rpg::getDamageBonus(bonus * 0.7f, actor.level) * 2);
	BOOST_CHECK_EQUAL(damage[rpg::DamageType::Blunt], 0u);
	BOOST_CHECK_EQUAL(damage[rpg::DamageType::Bullet], 0u);
	BOOST_CHECK_EQUAL(damage[rpg::DamageType::Magic], 0u);
	BOOST_CHECK_EQUAL(damage[rpg::DamageType::Fire], 0u);
	BOOST_CHECK_EQUAL(damage[rpg::DamageType::Ice], 0u);
	BOOST_CHECK_EQUAL(damage[rpg::DamageType::Poison], rpg::getDamageBonus(bonus * 0.3f, actor.level) * 2);
}

BOOST_AUTO_TEST_CASE(getWeaponDamage_adds_melee_bonus_to_blunt_for_fist_fighting) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto& actor = fix.addAvatar();
	auto damage = rpg::combat_impl::getWeaponDamage(actor, nullptr, nullptr);
	auto bonus = actor.properties[rpg::Property::MeleeBase];
	BOOST_CHECK_EQUAL(damage[rpg::DamageType::Blade], 0u);
	BOOST_CHECK_EQUAL(damage[rpg::DamageType::Blunt], rpg::getDamageBonus(bonus, actor.level));
	BOOST_CHECK_EQUAL(damage[rpg::DamageType::Bullet], 0u);
	BOOST_CHECK_EQUAL(damage[rpg::DamageType::Magic], 0u);
	BOOST_CHECK_EQUAL(damage[rpg::DamageType::Fire], 0u);
	BOOST_CHECK_EQUAL(damage[rpg::DamageType::Ice], 0u);
	BOOST_CHECK_EQUAL(damage[rpg::DamageType::Poison], 0u);
}

BOOST_AUTO_TEST_CASE(getWeaponDamage_adds_melee_bonus_for_single_melee_weapon) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto& actor = fix.addAvatar();
	auto damage = rpg::combat_impl::getWeaponDamage(actor, &fix.weapon, nullptr);
	auto expect = rpg::combat_impl::getWeaponDamage(actor, fix.weapon);
	BOOST_CHECK(damage == expect);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(getPerkDamage_adds_magic_bonus_with_level_to_perk) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto& actor = fix.addAvatar();
	auto damage = rpg::combat_impl::getPerkDamage(fix.perk.query(actor.id), actor, fix.spell);
	auto bonus = actor.properties[rpg::Property::MagicBase];
	BOOST_CHECK_EQUAL(damage[rpg::DamageType::Blade], 0u);
	BOOST_CHECK_EQUAL(damage[rpg::DamageType::Blunt], 0u);
	BOOST_CHECK_EQUAL(damage[rpg::DamageType::Bullet], 0u);
	BOOST_CHECK_EQUAL(damage[rpg::DamageType::Magic], rpg::getPerkBonus(0.1f, bonus, actor.level));
	BOOST_CHECK_EQUAL(damage[rpg::DamageType::Fire], 0u);
	BOOST_CHECK_EQUAL(damage[rpg::DamageType::Ice], rpg::getPerkBonus(0.9f, bonus, actor.level));
	BOOST_CHECK_EQUAL(damage[rpg::DamageType::Poison], 0u);
}

BOOST_AUTO_TEST_CASE(getPerkRecovery_adds_magic_bonus_with_level_to_perk) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto& actor = fix.addAvatar();
	auto recover = rpg::combat_impl::getPerkRecovery(fix.perk.query(actor.id), actor, fix.spell2);
	auto bonus = actor.properties[rpg::Property::MagicBase];
	auto life_gain = rpg::getPerkBonus(2.f, bonus, 10);
	auto stamina_gain = rpg::getPerkBonus(0.5f, bonus, 10);
	BOOST_CHECK_EQUAL(recover[rpg::Stat::Life], life_gain);
	BOOST_CHECK_EQUAL(recover[rpg::Stat::Mana], 0u);
	BOOST_CHECK_EQUAL(recover[rpg::Stat::Stamina], stamina_gain);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(getDamage_calculates_melee_weapon_damage) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto& actor = fix.addAvatar();
	auto& target = fix.addAvatar();
	auto data = fix.getMetaData(fix.weapon);
	auto damage = rpg::combat_impl::getDamage(fix.context, data, &actor, target);
	auto expected =
		rpg::combat_impl::getWeaponDamage(actor, &fix.weapon, nullptr);

	for (auto const& pair : damage) {
		BOOST_REQUIRE_EQUAL(pair.second, expected[pair.first]);
	}
}

BOOST_AUTO_TEST_CASE(getDamage_calculates_two_melee_weapon_damage) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto& actor = fix.addAvatar();
	auto& target = fix.addAvatar();
	rpg::CombatEvent event;
	auto data = fix.getMetaData(fix.weapon, fix.weapon);
	auto damage = rpg::combat_impl::getDamage(fix.context, data, &actor, target);
	auto expected = rpg::combat_impl::getWeaponDamage(actor, &fix.weapon, &fix.weapon);

	for (auto const& pair : damage) {
		BOOST_REQUIRE_EQUAL(pair.second, expected[pair.first]);
	}
}

BOOST_AUTO_TEST_CASE(getDamage_calculates_range_weapon_damage) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto& actor = fix.addAvatar();
	auto& target = fix.addAvatar();
	auto data = fix.getMetaData(fix.weapon2);
	auto damage = rpg::combat_impl::getDamage(fix.context, data, &actor, target);
	auto expected = rpg::combat_impl::getWeaponDamage(actor, &fix.weapon2, nullptr);

	for (auto const& pair : damage) {
		BOOST_REQUIRE_EQUAL(pair.second, expected[pair.first]);
	}
}

BOOST_AUTO_TEST_CASE(getDamage_calculates_perk_damage) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto& actor = fix.addAvatar();
	auto& target = fix.addAvatar();
	auto meta = fix.getMetaData(fix.spell);
	auto damage = rpg::combat_impl::getDamage(fix.context, meta, &actor, target);
	auto expected = rpg::combat_impl::getPerkDamage(
		fix.perk.query(actor.id), actor, fix.spell);

	for (auto const& pair : damage) {
		BOOST_REQUIRE_EQUAL(pair.second, expected[pair.first]);
	}
}

BOOST_AUTO_TEST_CASE(getDamage_calculates_effect_damage) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();
	
	fix.effect.damage[rpg::DamageType::Blade] = 20;
	fix.effect.damage[rpg::DamageType::Magic] = 13;

	auto& actor = fix.addAvatar();
	actor.level = 3u;
	auto meta = fix.getMetaData(fix.effect);
	auto damage = rpg::combat_impl::getDamage(fix.context, meta, nullptr, actor);
	auto expected = rpg::combat_impl::getEffectDamage(actor, fix.effect);

	for (auto const& pair : damage) {
		BOOST_REQUIRE_EQUAL(pair.second, expected[pair.first]);
		BOOST_CHECK_EQUAL(pair.second, rpg::getEffectValue(fix.effect.damage[pair.first], actor.level));
	}
}

BOOST_AUTO_TEST_CASE(getRecovery_calculates_perk_recovery) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto& actor = fix.addAvatar();
	auto& target = fix.addAvatar();
	auto meta = fix.getMetaData(fix.spell2);
	auto recovery = rpg::combat_impl::getRecovery(fix.context, meta, &actor, target);
	auto expected = rpg::combat_impl::getPerkRecovery(
		fix.perk.query(actor.id), actor, fix.spell2);

	for (auto const& pair : recovery) {
		BOOST_REQUIRE_EQUAL(pair.second, expected[pair.first]);
	}
}

BOOST_AUTO_TEST_CASE(getRecovery_calculates_effect_recovery) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	fix.effect.recover[rpg::Stat::Life] = -0.1f;
	fix.effect.recover[rpg::Stat::Stamina] = 0.05f;

	auto& actor = fix.addAvatar();
	actor.level = 3u;
	auto meta = fix.getMetaData(fix.effect);
	auto recovery = rpg::combat_impl::getRecovery(fix.context, meta, nullptr, actor);

	decltype(recovery) expect;
	expect[rpg::Stat::Life] = -0.1f * actor.properties[rpg::Property::MaxLife];
	expect[rpg::Stat::Mana] = 0.f;
	expect[rpg::Stat::Stamina] = 0.05f * actor.properties[rpg::Property::MaxStamina];
	
	for (auto const& pair : recovery) {
		BOOST_REQUIRE_EQUAL(pair.second, expect[pair.first]);
	}
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(getEffectEmitters_works_for_single_weapon) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto meta = fix.getMetaData(fix.weapon);
	auto emitters = rpg::combat_impl::getEffectEmitters(meta);

	BOOST_REQUIRE_EQUAL(emitters.size(), 1u);
	BOOST_CHECK_EQUAL(emitters[0].effect, &fix.effect);
	BOOST_CHECK_CLOSE(emitters[0].ratio, 1.f, 0.0001f);
}

BOOST_AUTO_TEST_CASE(getEffectEmitters_works_for_two_weapons) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto meta = fix.getMetaData(fix.weapon, fix.weapon);
	auto emitters = rpg::combat_impl::getEffectEmitters(meta);

	BOOST_REQUIRE_EQUAL(emitters.size(), 2u);
	BOOST_CHECK_EQUAL(emitters[0].effect, &fix.effect);
	BOOST_CHECK_CLOSE(emitters[0].ratio, 1.f, 0.0001f);
	BOOST_CHECK_EQUAL(emitters[1].effect, &fix.effect);
	BOOST_CHECK_CLOSE(emitters[1].ratio, 1.f, 0.0001f);
}

BOOST_AUTO_TEST_CASE(getEffectEmitters_works_for_perk) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto meta = fix.getMetaData(fix.spell);
	auto emitters = rpg::combat_impl::getEffectEmitters(meta);

	BOOST_REQUIRE_EQUAL(emitters.size(), 1u);
	BOOST_CHECK_EQUAL(emitters[0].effect, &fix.effect);
	BOOST_CHECK_CLOSE(emitters[0].ratio, 1.f, 0.0001f);
}

BOOST_AUTO_TEST_CASE(getEffectEmitters_works_for_trap) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto meta = fix.getMetaData(fix.trap);
	auto emitters = rpg::combat_impl::getEffectEmitters(meta);

	BOOST_REQUIRE_EQUAL(emitters.size(), 1u);
	BOOST_CHECK_EQUAL(emitters[0].effect, &fix.effect);
	BOOST_CHECK_CLOSE(emitters[0].ratio, 1.f, 0.0001f);
}

BOOST_AUTO_TEST_CASE(getEffectEmitters_returns_nothing_for_effects) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto meta = fix.getMetaData(fix.effect);
	auto emitters = rpg::combat_impl::getEffectEmitters(meta);

	BOOST_CHECK(emitters.empty());
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(onCombat_is_skipped_if_actors_target_is_dead) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto& actor = fix.addAvatar();
	auto& target = fix.addAvatar();
	target.stats[rpg::Stat::Life] = 0u;
	auto meta = fix.getMetaData(fix.weapon);
	rpg::CombatEvent event;
	event.actor = actor.id;
	event.target = target.id;
	event.meta_data = meta;
	rpg::combat_impl::onCombat(fix.context, event);

	BOOST_CHECK(fix.stats_sender.data().empty());
}

BOOST_AUTO_TEST_CASE(onCombat_is_skipped_if_bullets_target_is_dead) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto meta = fix.getMetaData(fix.trap);
	auto& actor = fix.addAvatar();
	auto proj = fix.addBullet(actor.id, meta);
	auto& target = fix.addAvatar();
	target.stats[rpg::Stat::Life] = 0u;

	rpg::CombatEvent event;
	event.actor = proj;
	event.target = target.id;
	event.meta_data = meta;
	rpg::combat_impl::onCombat(fix.context, event);

	BOOST_CHECK(fix.stats_sender.data().empty());
}

BOOST_AUTO_TEST_CASE(onCombat_is_skipped_if_actor_is_dead) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto& actor = fix.addAvatar();
	actor.stats[rpg::Stat::Life] = 0u;
	auto& target = fix.addAvatar();
	auto meta = fix.getMetaData(fix.weapon);
	rpg::CombatEvent event;
	event.actor = actor.id;
	event.target = target.id;
	event.meta_data = meta;
	rpg::combat_impl::onCombat(fix.context, event);

	BOOST_CHECK(fix.stats_sender.data().empty());
}

BOOST_AUTO_TEST_CASE(onCombat_is_not_skipped_if_bullets_owner_is_dead) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto& actor = fix.addAvatar();
	actor.stats[rpg::Stat::Life] = 0u;
	auto& target = fix.addAvatar();
	auto meta = fix.getMetaData(fix.weapon2);
	auto proj = fix.addBullet(actor.id, meta);
	rpg::CombatEvent event;
	event.actor = proj;
	event.target = target.id;
	event.meta_data = meta;
	rpg::combat_impl::onCombat(fix.context, event);

	BOOST_CHECK(!fix.stats_sender.data().empty());
}

BOOST_AUTO_TEST_CASE(onCombat_is_skipped_if_too_far_away_for_melee_combat) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto& actor = fix.addAvatar();
	auto& target = fix.addAvatar();
	auto& body = fix.movement.query(target.id);
	body.pos.x += 2.01f;
	auto meta = fix.getMetaData(fix.weapon);
	rpg::CombatEvent event;
	event.actor = actor.id;
	event.target = target.id;
	event.meta_data = meta;
	rpg::combat_impl::onCombat(fix.context, event);

	BOOST_CHECK(fix.stats_sender.data().empty());
}

BOOST_AUTO_TEST_CASE(onCombat_is_calculated_if_avatars_are_far_but_not_too_far_away) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto& actor = fix.addAvatar();
	auto& target = fix.addAvatar();
	auto& body = fix.movement.query(target.id);
	body.pos.x += 1.99f;
	auto meta = fix.getMetaData(fix.weapon);
	rpg::CombatEvent event;
	event.actor = actor.id;
	event.target = target.id;
	event.meta_data = meta;
	rpg::combat_impl::onCombat(fix.context, event);

	BOOST_CHECK(!fix.stats_sender.data().empty());
}

BOOST_AUTO_TEST_CASE(
	onCombat_is_not_skipped_if_melee_target_is_near_but_diagonally) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto& actor = fix.addAvatar();
	auto& target = fix.addAvatar();
	auto& body = fix.movement.query(target.id);
	body.pos.x += 1.f;
	body.pos.y += 1.f;
	auto meta = fix.getMetaData(fix.weapon);
	rpg::CombatEvent event;
	event.actor = actor.id;
	event.target = target.id;
	event.meta_data = meta;
	rpg::combat_impl::onCombat(fix.context, event);

	BOOST_CHECK(!fix.stats_sender.data().empty());
}

BOOST_AUTO_TEST_CASE(onCombat_inflicts_damage_via_stats_event) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto& actor = fix.addAvatar();
	auto& target = fix.addAvatar();
	auto meta = fix.getMetaData(fix.weapon);
	rpg::CombatEvent event;
	event.actor = actor.id;
	event.target = target.id;
	event.meta_data = meta;
	rpg::combat_impl::onCombat(fix.context, event);
	
	auto dmg = rpg::combat_impl::getWeaponDamage(actor, fix.weapon);
	auto def = rpg::combat_impl::getDefense(target);
	int delta{0};
	for (auto const & pair: dmg) {
		auto tmp = def[pair.first];
		if (pair.second > tmp) {
			delta += pair.second - tmp;
		}
	}
	
	auto const& stats = fix.stats_sender.data();
	BOOST_REQUIRE_EQUAL(stats.size(), 1u);
	BOOST_CHECK_EQUAL(stats[0].actor, target.id);
	BOOST_CHECK_EQUAL(stats[0].delta[rpg::Stat::Life], -delta);
	BOOST_CHECK_EQUAL(stats[0].delta[rpg::Stat::Mana], 0);
	BOOST_CHECK_EQUAL(stats[0].delta[rpg::Stat::Stamina], 0);
}

BOOST_AUTO_TEST_CASE(onCombat_triggers_stats_event_with_causer_if_weapon_used) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto& actor = fix.addAvatar();
	auto& target = fix.addAvatar();
	auto meta = fix.getMetaData(fix.weapon);
	rpg::CombatEvent event;
	event.actor = actor.id;
	event.target = target.id;
	event.meta_data = meta;
	rpg::combat_impl::onCombat(fix.context, event);

	auto const& stats = fix.stats_sender.data();
	BOOST_REQUIRE_EQUAL(stats.size(), 1u);
	BOOST_CHECK_EQUAL(stats[0].causer, actor.id);
}

BOOST_AUTO_TEST_CASE(
	onCombat_triggers_stats_event_with_causer_if_weapons_bullet_used) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto& actor = fix.addAvatar();
	auto& target = fix.addAvatar();
	auto meta = fix.getMetaData(fix.weapon2);
	auto proj = fix.addBullet(actor.id, meta);
	rpg::CombatEvent event;
	event.actor = proj;
	event.target = target.id;
	event.meta_data = meta;
	rpg::combat_impl::onCombat(fix.context, event);

	auto const& stats = fix.stats_sender.data();
	BOOST_REQUIRE_EQUAL(stats.size(), 1u);
	BOOST_CHECK_EQUAL(stats[0].causer, actor.id);
}

BOOST_AUTO_TEST_CASE(onCombat_triggers_stats_event_with_causer_if_perk_used) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto& actor = fix.addAvatar();
	auto& target = fix.addAvatar();
	auto meta = fix.getMetaData(fix.spell);
	rpg::CombatEvent event;
	event.actor = actor.id;
	event.target = target.id;
	event.meta_data = meta;
	rpg::combat_impl::onCombat(fix.context, event);

	auto const& stats = fix.stats_sender.data();
	BOOST_REQUIRE_EQUAL(stats.size(), 1u);
	BOOST_CHECK_EQUAL(stats[0].causer, actor.id);
}

BOOST_AUTO_TEST_CASE(
	onCombat_triggers_stats_event_with_causer_if_perk_bullet_used) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto& actor = fix.addAvatar();
	auto& target = fix.addAvatar();
	auto meta = fix.getMetaData(fix.spell2);
	auto proj = fix.addBullet(actor.id, meta);
	rpg::CombatEvent event;
	event.actor = proj;
	event.target = target.id;
	event.meta_data = meta;
	rpg::combat_impl::onCombat(fix.context, event);

	auto const& stats = fix.stats_sender.data();
	BOOST_REQUIRE_EQUAL(stats.size(), 1u);
	BOOST_CHECK_EQUAL(stats[0].causer, actor.id);
}

BOOST_AUTO_TEST_CASE(
	onCombat_triggers_stats_event_without_causer_if_trap_used) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto& target = fix.addAvatar();
	auto meta = fix.getMetaData(fix.trap);
	auto proj = fix.addBullet(0u, meta);
	rpg::CombatEvent event;
	event.actor = proj;
	event.target = target.id;
	event.meta_data = meta;
	rpg::combat_impl::onCombat(fix.context, event);

	auto const& stats = fix.stats_sender.data();
	BOOST_REQUIRE_EQUAL(stats.size(), 1u);
	BOOST_CHECK_EQUAL(stats[0].causer, 0u);
}

BOOST_AUTO_TEST_CASE(
	onCombat_triggers_stats_event_without_causer_if_damaging_effect_used) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();
	
	fix.effect.damage[rpg::DamageType::Magic] = 100;
	
	auto& target = fix.addAvatar();
	auto meta = fix.getMetaData(fix.effect);
	rpg::CombatEvent event;
	event.actor = 0u;
	event.target = target.id;
	event.meta_data = meta;
	rpg::combat_impl::onCombat(fix.context, event);

	auto const& stats = fix.stats_sender.data();
	BOOST_REQUIRE_EQUAL(stats.size(), 1u);
	BOOST_CHECK_EQUAL(stats[0].causer, 0u);
	BOOST_CHECK(stats[0].delta[rpg::Stat::Life] < 0);
}

BOOST_AUTO_TEST_CASE(
	onCombat_triggers_stats_event_without_causer_if_recover_effect_used) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();
	
	fix.effect.recover[rpg::Stat::Life] = 10;
	
	auto& target = fix.addAvatar();
	auto meta = fix.getMetaData(fix.effect);
	rpg::CombatEvent event;
	event.actor = 0u;
	event.target = target.id;
	event.meta_data = meta;
	rpg::combat_impl::onCombat(fix.context, event);

	auto const& stats = fix.stats_sender.data();
	BOOST_REQUIRE_EQUAL(stats.size(), 1u);
	BOOST_CHECK_EQUAL(stats[0].causer, 0u);
	BOOST_CHECK(stats[0].delta[rpg::Stat::Life] > 0);
}

BOOST_AUTO_TEST_CASE(onCombat_causes_exp_gain_if_avatar_attacking) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto& actor = fix.addAvatar();
	auto& target = fix.addAvatar();
	auto meta = fix.getMetaData(fix.weapon);
	rpg::CombatEvent event;
	event.actor = actor.id;
	event.target = target.id;
	event.meta_data = meta;
	rpg::combat_impl::onCombat(fix.context, event);

	auto const& stats = fix.stats_sender.data();
	BOOST_REQUIRE_EQUAL(stats.size(), 1u);
	
	auto expected = rpg::getExpGain(-stats[0].delta[rpg::Stat::Life], actor.level);
	
	auto const& exp = fix.exp_sender.data();
	BOOST_REQUIRE_EQUAL(exp.size(), 1u);
	BOOST_CHECK_EQUAL(exp[0].actor, actor.id);
	BOOST_CHECK_EQUAL(exp[0].exp, expected);
}

BOOST_AUTO_TEST_CASE(onCombat_causes_exp_gain_if_avatar_attacking_via_bullet) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto& actor = fix.addAvatar();
	auto& target = fix.addAvatar();
	auto meta = fix.getMetaData(fix.spell);
	auto proj = fix.addBullet(actor.id, meta);
	rpg::CombatEvent event;
	event.actor = proj;
	event.target = target.id;
	event.meta_data = meta;
	rpg::combat_impl::onCombat(fix.context, event);

	auto const& stats = fix.stats_sender.data();
	BOOST_REQUIRE_EQUAL(stats.size(), 1u);

	auto expected = rpg::getExpGain(-stats[0].delta[rpg::Stat::Life], actor.level);
	
	auto const& exp = fix.exp_sender.data();
	BOOST_REQUIRE_EQUAL(exp.size(), 1u);
	BOOST_CHECK_EQUAL(exp[0].actor, actor.id);
	BOOST_CHECK_EQUAL(exp[0].exp, expected);
}

BOOST_AUTO_TEST_CASE(onCombat_causes_no_exp_gain_if_no_actor) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto& target = fix.addAvatar();
	auto meta = fix.getMetaData(fix.trap);
	rpg::CombatEvent event;
	event.actor = 0u;
	event.target = target.id;
	event.meta_data = meta;
	rpg::combat_impl::onCombat(fix.context, event);

	BOOST_CHECK(fix.exp_sender.data().empty());
}

BOOST_AUTO_TEST_CASE(onCombat_inflicts_effects_by_both_weapons) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto& actor = fix.addAvatar();
	auto& target = fix.addAvatar();
	auto meta = fix.getMetaData(fix.weapon, fix.weapon);
	rpg::CombatEvent event;
	event.actor = actor.id;
	event.target = target.id;
	event.meta_data = meta;
	rpg::combat_impl::onCombat(fix.context, event);

	auto const& effects = fix.effect_sender.data();
	BOOST_REQUIRE_EQUAL(effects.size(), 2u);
	BOOST_CHECK_EQUAL(effects[0].actor, target.id);
	BOOST_CHECK_EQUAL(effects[0].causer, actor.id);
	BOOST_CHECK_EQUAL(effects[0].effect, &fix.effect);
	BOOST_CHECK(effects[0].type == rpg::EffectEvent::Add);
	BOOST_CHECK_EQUAL(effects[1].actor, target.id);
	BOOST_CHECK_EQUAL(effects[1].causer, actor.id);
	BOOST_CHECK_EQUAL(effects[1].effect, &fix.effect);
	BOOST_CHECK(effects[1].type == rpg::EffectEvent::Add);
}

BOOST_AUTO_TEST_CASE(onCombat_inflicts_effects_by_bow) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto& actor = fix.addAvatar();
	auto& target = fix.addAvatar();
	auto meta = fix.getMetaData(fix.weapon2);
	auto proj = fix.addBullet(actor.id, meta);
	rpg::CombatEvent event;
	event.actor = proj;
	event.target = target.id;
	event.meta_data = meta;
	rpg::combat_impl::onCombat(fix.context, event);

	auto const& effects = fix.effect_sender.data();
	BOOST_REQUIRE_EQUAL(effects.size(), 1u);
	BOOST_CHECK_EQUAL(effects[0].actor, target.id);
	BOOST_CHECK_EQUAL(effects[0].causer, actor.id);
	BOOST_CHECK_EQUAL(effects[0].effect, &fix.effect);
}

BOOST_AUTO_TEST_CASE(onCombat_inflicts_effects_by_perk) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto& actor = fix.addAvatar();
	auto& target = fix.addAvatar();
	auto meta = fix.getMetaData(fix.spell);
	auto proj = fix.addBullet(actor.id, meta);
	rpg::CombatEvent event;
	event.actor = proj;
	event.target = target.id;
	event.meta_data = meta;
	rpg::combat_impl::onCombat(fix.context, event);

	auto const& effects = fix.effect_sender.data();
	BOOST_REQUIRE_EQUAL(effects.size(), 1u);
	BOOST_CHECK_EQUAL(effects[0].actor, target.id);
	BOOST_CHECK_EQUAL(effects[0].causer, actor.id);
	BOOST_CHECK_EQUAL(effects[0].effect, &fix.effect);
}

BOOST_AUTO_TEST_CASE(onCombat_marks_projectile_for_destruction_after_combat) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto& actor = fix.addAvatar();
	auto meta = fix.getMetaData(fix.spell);
	auto proj = fix.addBullet(actor.id, meta);
	auto& target = fix.addAvatar();
	rpg::CombatEvent event;
	event.actor = proj;
	event.target = target.id;
	event.meta_data = meta;
	rpg::combat_impl::onCombat(fix.context, event);

	BOOST_REQUIRE_EQUAL(fix.context.projectiles.size(), 1u);
	BOOST_CHECK_EQUAL(fix.context.projectiles[0], proj);
}

BOOST_AUTO_TEST_CASE(
	onCombat_marks_projectile_for_destruction_when_barrier_is_hit) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto& actor = fix.addAvatar();
	auto meta = fix.getMetaData(fix.spell);
	auto proj = fix.addBullet(actor.id, meta);
	auto& target = fix.addInteractable();
	target.type = rpg::InteractType::Barrier;
	rpg::CombatEvent event;
	event.actor = proj;
	event.target = target.id;
	event.meta_data = meta;
	rpg::combat_impl::onCombat(fix.context, event);

	BOOST_REQUIRE_EQUAL(fix.context.projectiles.size(), 1u);
	BOOST_CHECK_EQUAL(fix.context.projectiles[0], proj);
}

BOOST_AUTO_TEST_CASE(
	onCombat_does_not_mark_projectile_for_destruction_when_corpse_is_hit) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto& actor = fix.addAvatar();
	auto meta = fix.getMetaData(fix.spell);
	auto proj = fix.addBullet(actor.id, meta);
	auto& target = fix.addInteractable();
	target.type = rpg::InteractType::Corpse;
	rpg::CombatEvent event;
	event.actor = proj;
	event.target = target.id;
	event.meta_data = meta;
	rpg::combat_impl::onCombat(fix.context, event);

	BOOST_REQUIRE_EQUAL(fix.context.projectiles.size(), 0u);
}

BOOST_AUTO_TEST_CASE(onCombat_marks_projectile_only_once) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto meta = fix.getMetaData(fix.trap);
	auto proj = fix.addBullet(0u, meta);
	fix.context.projectiles.push_back(proj);
	auto& target = fix.addAvatar();
	rpg::CombatEvent event;
	event.actor = proj;
	event.target = target.id;
	event.meta_data = meta;
	rpg::combat_impl::onCombat(fix.context, event);

	BOOST_REQUIRE_EQUAL(fix.context.projectiles.size(), 1u);
	BOOST_CHECK_EQUAL(fix.context.projectiles[0], proj);
}

BOOST_AUTO_TEST_CASE(
	onCombat_triggers_respawn_if_perk_can_revive_and_target_is_dead) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto& actor = fix.addAvatar();
	auto& target = fix.addAvatar();
	target.stats[rpg::Stat::Life] = 0u;
	auto meta = fix.getMetaData(fix.revive);

	rpg::CombatEvent event;
	event.actor = actor.id;
	event.target = target.id;
	event.meta_data = meta;
	rpg::combat_impl::onCombat(fix.context, event);

	auto const& respawns = fix.context.respawn_sender.data();
	BOOST_REQUIRE_EQUAL(respawns.size(), 1u);
	BOOST_CHECK_EQUAL(respawns[0].actor, target.id);
	BOOST_CHECK_EQUAL(respawns[0].causer, actor.id);
}

BOOST_AUTO_TEST_CASE(
	onCombat_does_not_trigger_respawn_if_perk_can_revive_but_target_is_alive) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto& actor = fix.addAvatar();
	auto& target = fix.addAvatar();
	target.stats[rpg::Stat::Life] = 1u;
	auto meta = fix.getMetaData(fix.revive);

	rpg::CombatEvent event;
	event.actor = actor.id;
	event.target = target.id;
	event.meta_data = meta;
	rpg::combat_impl::onCombat(fix.context, event);

	auto const& respawns = fix.context.respawn_sender.data();
	BOOST_REQUIRE_EQUAL(respawns.size(), 0u);
}

BOOST_AUTO_TEST_CASE(onCombat_triggers_heal_on_respawn) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto& actor = fix.addAvatar();
	auto& target = fix.addAvatar();
	target.stats[rpg::Stat::Life] = 0u;
	auto meta = fix.getMetaData(fix.revive);

	rpg::CombatEvent event;
	event.actor = actor.id;
	event.target = target.id;
	event.meta_data = meta;
	rpg::combat_impl::onCombat(fix.context, event);

	auto expect = rpg::combat_impl::getRecovery(fix.context, meta, &actor, target)[rpg::Stat::Life];
	auto const& stats = fix.context.stats_sender.data();
	BOOST_REQUIRE_EQUAL(stats.size(), 1u);
	BOOST_CHECK_EQUAL(stats[0].delta[rpg::Stat::Life], expect);
}

BOOST_AUTO_TEST_CASE(onCombat_triggers_exp_for_healing_on_respawn) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	auto& actor = fix.addAvatar();
	auto& target = fix.addAvatar();
	target.stats[rpg::Stat::Life] = 0u;
	auto meta = fix.getMetaData(fix.revive);

	rpg::CombatEvent event;
	event.actor = actor.id;
	event.target = target.id;
	event.meta_data = meta;
	rpg::combat_impl::onCombat(fix.context, event);

	auto expected = rpg::getExpGain(rpg::combat_impl::getRecovery(fix.context, meta, &actor, target)[rpg::Stat::Life], actor.level);
	auto const& exp = fix.context.exp_sender.data();
	BOOST_REQUIRE_EQUAL(exp.size(), 1u);
	BOOST_CHECK_EQUAL(exp[0].actor, actor.id);
	BOOST_CHECK_EQUAL(exp[0].exp, expected);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(
	onUpdate_triggers_projectile_destruction_per_marked_projectile) {
	auto& fix = Singleton<CombatFixture>::get();
	fix.reset();

	fix.context.projectiles.push_back(13u);
	fix.context.projectiles.push_back(6u);
	rpg::combat_impl::onUpdate(fix.context, sf::Time::Zero);
	auto const& events = fix.projectile_sender.data();

	BOOST_REQUIRE_EQUAL(events.size(), 2u);
	BOOST_CHECK_EQUAL(events[0].id, 13u);
	BOOST_CHECK(events[0].type == rpg::ProjectileEvent::Destroy);
	BOOST_CHECK_EQUAL(events[1].id, 6u);
	BOOST_CHECK(events[1].type == rpg::ProjectileEvent::Destroy);
}

BOOST_AUTO_TEST_SUITE_END()

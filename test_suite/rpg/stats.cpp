#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>
#include <testsuite/singleton.hpp>

#include <rpg/balance.hpp>
#include <rpg/stats.hpp>

struct StatsFixture {
	core::LogContext log;
	rpg::StatsSender stats_sender;
	rpg::DeathSender death_sender;

	rpg::stats_impl::Context context;
	rpg::StatsData actor;
	rpg::StatsBoni bonus, malus, mixed;

	StatsFixture()
		: log{}
		, stats_sender{}
		, death_sender{}
		, context{log, stats_sender, death_sender}
		, actor{}
		, bonus{}
		, malus{}
		, mixed{} {
		actor.id = 1u;

		bonus.properties[rpg::Property::MagicBase] = 15;
		bonus.defense[rpg::DamageType::Fire] = 10.f;
		malus.properties[rpg::Property::MeleeBase] = -20;
		malus.defense[rpg::DamageType::Ice] = -3.f;
		mixed.properties[rpg::Property::MagicBase] = 15;
		mixed.defense[rpg::DamageType::Ice] = -3.f;
	}

	void reset() {
		actor.godmode = false;
		actor.level = 40u;
		actor.attributes = decltype(actor.attributes){20};
		actor.prop_boni = decltype(actor.prop_boni){0};
		actor.base_def = decltype(actor.base_def){0.f};
		actor.stats = decltype(actor.stats){100};
		rpg::stats_impl::refresh(actor);

		stats_sender.clear();
		death_sender.clear();
		
		// clear logs
		log.debug.clear();
		log.warning.clear();
		log.error.clear();
	}
};

BOOST_AUTO_TEST_SUITE(stats_test)

BOOST_AUTO_TEST_CASE(stats_can_be_increased) {
	auto& fix = Singleton<StatsFixture>::get();
	fix.reset();

	rpg::StatsEvent event;
	event.delta[rpg::Stat::Life] = 10;
	event.delta[rpg::Stat::Mana] = 15;
	event.delta[rpg::Stat::Stamina] = 7;
	rpg::stats_impl::applyStats(fix.context, fix.actor, event);

	BOOST_CHECK_EQUAL(fix.actor.stats[rpg::Stat::Life], 110);
	BOOST_CHECK_EQUAL(fix.actor.stats[rpg::Stat::Mana], 115);
	BOOST_CHECK_EQUAL(fix.actor.stats[rpg::Stat::Stamina], 107);
}

BOOST_AUTO_TEST_CASE(stats_cannot_be_increased_if_actor_dead) {
	auto& fix = Singleton<StatsFixture>::get();
	fix.reset();

	fix.actor.stats[rpg::Stat::Life] = 0u;
	rpg::StatsEvent event;
	event.delta[rpg::Stat::Life] = 10;
	event.delta[rpg::Stat::Mana] = 15;
	event.delta[rpg::Stat::Stamina] = 7;
	BOOST_CHECK(!rpg::stats_impl::applyStats(fix.context, fix.actor, event));

	BOOST_CHECK_EQUAL(fix.actor.stats[rpg::Stat::Life], 0);
	BOOST_CHECK_EQUAL(fix.actor.stats[rpg::Stat::Mana], 100);
	BOOST_CHECK_EQUAL(fix.actor.stats[rpg::Stat::Stamina], 100);
}

BOOST_AUTO_TEST_CASE(stats_can_be_decreased) {
	auto& fix = Singleton<StatsFixture>::get();
	fix.reset();

	rpg::StatsEvent event;
	event.delta[rpg::Stat::Life] = -3;
	event.delta[rpg::Stat::Mana] = -15;
	event.delta[rpg::Stat::Stamina] = -8;
	rpg::stats_impl::applyStats(fix.context, fix.actor, event);

	BOOST_CHECK_EQUAL(fix.actor.stats[rpg::Stat::Life], 97);
	BOOST_CHECK_EQUAL(fix.actor.stats[rpg::Stat::Mana], 85);
	BOOST_CHECK_EQUAL(fix.actor.stats[rpg::Stat::Stamina], 92);
}

BOOST_AUTO_TEST_CASE(stats_can_be_changed_independently) {
	auto& fix = Singleton<StatsFixture>::get();
	fix.reset();

	rpg::StatsEvent event;
	event.delta[rpg::Stat::Life] = 15;
	event.delta[rpg::Stat::Mana] = -12;
	event.delta[rpg::Stat::Stamina] = 0;
	rpg::stats_impl::applyStats(fix.context, fix.actor, event);

	BOOST_CHECK_EQUAL(fix.actor.stats[rpg::Stat::Life], 115);
	BOOST_CHECK_EQUAL(fix.actor.stats[rpg::Stat::Mana], 88);
	BOOST_CHECK_EQUAL(fix.actor.stats[rpg::Stat::Stamina], 100);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(stats_cannot_underflow) {
	auto& fix = Singleton<StatsFixture>::get();
	fix.reset();

	rpg::StatsEvent event;
	event.delta[rpg::Stat::Life] = -101;
	event.delta[rpg::Stat::Mana] = -200;
	event.delta[rpg::Stat::Stamina] = -5000;
	rpg::stats_impl::applyStats(fix.context, fix.actor, event);

	BOOST_CHECK_EQUAL(fix.actor.stats[rpg::Stat::Life], 0);
	BOOST_CHECK_EQUAL(fix.actor.stats[rpg::Stat::Mana], 0);
	BOOST_CHECK_EQUAL(fix.actor.stats[rpg::Stat::Stamina], 0);
}

BOOST_AUTO_TEST_CASE(stats_cannot_overflow) {
	auto& fix = Singleton<StatsFixture>::get();
	fix.reset();

	fix.actor.stats[rpg::Stat::Life] =
		fix.actor.properties[rpg::Property::MaxLife];
	fix.actor.stats[rpg::Stat::Mana] =
		fix.actor.properties[rpg::Property::MaxMana];
	fix.actor.stats[rpg::Stat::Stamina] =
		fix.actor.properties[rpg::Property::MaxStamina];
	rpg::StatsEvent event;
	event.delta[rpg::Stat::Life] = 365;
	event.delta[rpg::Stat::Mana] = 200;
	event.delta[rpg::Stat::Stamina] = 5000;
	rpg::stats_impl::applyStats(fix.context, fix.actor, event);

	BOOST_CHECK_EQUAL(fix.actor.stats[rpg::Stat::Life],
		fix.actor.properties[rpg::Property::MaxLife]);
	BOOST_CHECK_EQUAL(fix.actor.stats[rpg::Stat::Mana],
		fix.actor.properties[rpg::Property::MaxMana]);
	BOOST_CHECK_EQUAL(fix.actor.stats[rpg::Stat::Stamina],
		fix.actor.properties[rpg::Property::MaxStamina]);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(death_is_propagated_if_life_is_decreased_to_zero) {
	auto& fix = Singleton<StatsFixture>::get();
	fix.reset();

	rpg::StatsEvent event;
	event.causer = 17u;
	event.delta[rpg::Stat::Life] = -120;
	rpg::stats_impl::applyStats(fix.context, fix.actor, event);

	auto const& events = fix.death_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, 1u);
	BOOST_CHECK_EQUAL(events[0].causer, 17u);
}

BOOST_AUTO_TEST_CASE(death_is_not_propagated_if_already_dead) {
	auto& fix = Singleton<StatsFixture>::get();
	fix.reset();

	fix.actor.stats[rpg::Stat::Life] = 0u;
	rpg::StatsEvent event;
	event.delta[rpg::Stat::Life] = -120;
	BOOST_CHECK(!rpg::stats_impl::applyStats(fix.context, fix.actor, event));

	BOOST_CHECK(fix.death_sender.data().empty());
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(stats_delta_is_not_changed_if_it_was_completly_applied) {
	auto& fix = Singleton<StatsFixture>::get();
	fix.reset();

	rpg::StatsEvent event;
	event.delta[rpg::Stat::Life] = 10;
	event.delta[rpg::Stat::Mana] = 15;
	event.delta[rpg::Stat::Stamina] = 7;
	BOOST_CHECK(rpg::stats_impl::applyStats(fix.context, fix.actor, event));

	BOOST_CHECK_EQUAL(event.delta[rpg::Stat::Life], 10);
	BOOST_CHECK_EQUAL(event.delta[rpg::Stat::Mana], 15);
	BOOST_CHECK_EQUAL(event.delta[rpg::Stat::Stamina], 7);
}

BOOST_AUTO_TEST_CASE(stats_delta_is_changed_if_it_wasnt_completly_applied) {
	auto& fix = Singleton<StatsFixture>::get();
	fix.reset();

	fix.actor.stats[rpg::Stat::Life] =
		fix.actor.properties[rpg::Property::MaxLife] - 10;
	fix.actor.stats[rpg::Stat::Mana] =
		fix.actor.properties[rpg::Property::MaxMana] - 10;
	fix.actor.stats[rpg::Stat::Stamina] = 0;
	rpg::StatsEvent event;
	event.delta[rpg::Stat::Life] = 10;
	event.delta[rpg::Stat::Mana] = 15;
	event.delta[rpg::Stat::Stamina] = -7;
	BOOST_CHECK(rpg::stats_impl::applyStats(fix.context, fix.actor, event));

	BOOST_CHECK_EQUAL(event.delta[rpg::Stat::Life], 10);
	BOOST_CHECK_EQUAL(event.delta[rpg::Stat::Mana], 10);
	BOOST_CHECK_EQUAL(event.delta[rpg::Stat::Stamina], 0);
}

BOOST_AUTO_TEST_CASE(stats_delta_is_reset_if_it_wasnt_applied_at_all) {
	auto& fix = Singleton<StatsFixture>::get();
	fix.reset();

	fix.actor.stats[rpg::Stat::Life] =
		fix.actor.properties[rpg::Property::MaxLife];
	fix.actor.stats[rpg::Stat::Mana] = 0;
	fix.actor.stats[rpg::Stat::Stamina] =
		fix.actor.properties[rpg::Property::MaxStamina];
	rpg::StatsEvent event;
	event.delta[rpg::Stat::Life] = 0;
	event.delta[rpg::Stat::Mana] = -3;
	event.delta[rpg::Stat::Stamina] = 7;
	rpg::stats_impl::applyStats(fix.context, fix.actor, event);

	BOOST_CHECK_EQUAL(event.delta[rpg::Stat::Life], 0);
	BOOST_CHECK_EQUAL(event.delta[rpg::Stat::Mana], 0);
	BOOST_CHECK_EQUAL(event.delta[rpg::Stat::Stamina], 0);
}

BOOST_AUTO_TEST_CASE(stats_delta_is_not_applied_if_godmode) {
	auto& fix = Singleton<StatsFixture>::get();
	fix.reset();
	
	fix.actor.godmode = true;
	fix.actor.stats[rpg::Stat::Life] = fix.actor.properties[rpg::Property::MaxLife] / 2;
	fix.actor.stats[rpg::Stat::Mana] = fix.actor.properties[rpg::Property::MaxMana] / 2;
	fix.actor.stats[rpg::Stat::Stamina] = fix.actor.properties[rpg::Property::MaxStamina] / 2;
	rpg::StatsEvent event;
	event.delta[rpg::Stat::Life] = 5;
	event.delta[rpg::Stat::Mana] = -6;
	event.delta[rpg::Stat::Stamina] = 7;
	rpg::stats_impl::applyStats(fix.context, fix.actor, event);

	BOOST_CHECK_EQUAL(event.delta[rpg::Stat::Life], 0);
	BOOST_CHECK_EQUAL(event.delta[rpg::Stat::Mana], 0);
	BOOST_CHECK_EQUAL(event.delta[rpg::Stat::Stamina], 0);

	BOOST_CHECK_EQUAL(fix.actor.stats[rpg::Stat::Life], fix.actor.properties[rpg::Property::MaxLife] / 2);
	BOOST_CHECK_EQUAL(fix.actor.stats[rpg::Stat::Mana], fix.actor.properties[rpg::Property::MaxMana] / 2);
	BOOST_CHECK_EQUAL(fix.actor.stats[rpg::Stat::Stamina], fix.actor.properties[rpg::Property::MaxStamina] / 2);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(boni_can_be_added) {
	auto& fix = Singleton<StatsFixture>::get();
	fix.reset();

	auto base_magic = fix.actor.base_props[rpg::Property::MagicBase];

	rpg::stats_impl::addBoni(fix.context, fix.actor, fix.bonus);

	BOOST_CHECK_EQUAL(fix.actor.base_props[rpg::Property::MagicBase], base_magic);
	BOOST_CHECK_EQUAL(fix.actor.prop_boni[rpg::Property::MagicBase], fix.bonus.properties[rpg::Property::MagicBase]);
	BOOST_CHECK_EQUAL(fix.actor.properties[rpg::Property::MagicBase], base_magic + fix.bonus.properties[rpg::Property::MagicBase]);
	BOOST_CHECK_CLOSE(fix.actor.base_def[rpg::DamageType::Fire], fix.bonus.defense[rpg::DamageType::Fire], 0.0001f);
}

BOOST_AUTO_TEST_CASE(boni_can_be_removed) {
	auto& fix = Singleton<StatsFixture>::get();
	fix.reset();

	auto base_magic = fix.actor.base_props[rpg::Property::MagicBase];

	rpg::stats_impl::addBoni(fix.context, fix.actor, fix.bonus);
	rpg::stats_impl::removeBoni(fix.context, fix.actor, fix.bonus);

	BOOST_CHECK_EQUAL(fix.actor.base_props[rpg::Property::MagicBase], base_magic);
	BOOST_CHECK_EQUAL(fix.actor.prop_boni[rpg::Property::MagicBase], 0u);
	BOOST_CHECK_EQUAL(fix.actor.properties[rpg::Property::MagicBase], base_magic);
	BOOST_CHECK_CLOSE(fix.actor.base_def[rpg::DamageType::Fire], 0.f, 0.0001f);
}

BOOST_AUTO_TEST_CASE(mali_can_be_added) {
	auto& fix = Singleton<StatsFixture>::get();
	fix.reset();

	rpg::stats_impl::refresh(fix.actor);
	int melee_base = fix.actor.base_props[rpg::Property::MeleeBase];

	fix.actor.base_def[rpg::DamageType::Ice] = 5.f;
	rpg::stats_impl::addBoni(fix.context, fix.actor, fix.malus);

	BOOST_CHECK_EQUAL(fix.actor.base_props[rpg::Property::MeleeBase], melee_base);
	BOOST_CHECK_EQUAL(fix.actor.prop_boni[rpg::Property::MeleeBase], fix.malus.properties[rpg::Property::MeleeBase]);
	BOOST_CHECK_EQUAL(fix.actor.properties[rpg::Property::MeleeBase], 0u); // because negative properties are ignored while calculation
	BOOST_CHECK_CLOSE(fix.actor.base_def[rpg::DamageType::Ice], 2.f, 0.0001f);
}

BOOST_AUTO_TEST_CASE(mali_can_be_removed) {
	auto& fix = Singleton<StatsFixture>::get();
	fix.reset();
	
	int melee_base = fix.actor.base_props[rpg::Property::MeleeBase];
	
	fix.actor.base_def[rpg::DamageType::Ice] = 5.f;
	rpg::stats_impl::addBoni(fix.context, fix.actor, fix.malus);
	rpg::stats_impl::removeBoni(fix.context, fix.actor, fix.malus);

	BOOST_CHECK_EQUAL(fix.actor.base_props[rpg::Property::MeleeBase], melee_base);
	BOOST_CHECK_EQUAL(fix.actor.prop_boni[rpg::Property::MeleeBase], 0);
	BOOST_CHECK_EQUAL(fix.actor.properties[rpg::Property::MeleeBase], melee_base);
	BOOST_CHECK_CLOSE(fix.actor.base_def[rpg::DamageType::Ice], 5.f, 0.0001f);
}

BOOST_AUTO_TEST_CASE(temporary_negative_defense_is_possible) {
	auto& fix = Singleton<StatsFixture>::get();
	fix.reset();

	fix.actor.base_def[rpg::DamageType::Ice] = 1.f;
	rpg::stats_impl::addBoni(fix.context, fix.actor, fix.malus);

	BOOST_CHECK_CLOSE(fix.actor.base_def[rpg::DamageType::Ice], -2.f, 0.0001f);

	rpg::stats_impl::removeBoni(fix.context, fix.actor, fix.malus);

	BOOST_CHECK_CLOSE(fix.actor.base_def[rpg::DamageType::Ice], 1.f, 0.0001f);
}

BOOST_AUTO_TEST_CASE(temporary_negative_property_is_ignored) {
	auto& fix = Singleton<StatsFixture>::get();
	fix.reset();

	rpg::stats_impl::refresh(fix.actor);
	BOOST_CHECK(fix.actor.base_props[rpg::Property::MeleeBase] > 0);
	
	rpg::stats_impl::addBoni(fix.context, fix.actor, fix.malus);
	BOOST_CHECK_EQUAL(fix.actor.properties[rpg::Property::MeleeBase], 0u);

	rpg::stats_impl::removeBoni(fix.context, fix.actor, fix.malus);
	BOOST_CHECK_EQUAL(fix.actor.properties[rpg::Property::MeleeBase], rpg::getMeleeBase(fix.actor.attributes, fix.actor.level));
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(train_attribute_increases_given_attribute_by_one) {
	auto& fix = Singleton<StatsFixture>::get();
	fix.reset();

	rpg::stats_impl::increaseAttribute(
		fix.context, fix.actor, rpg::Attribute::Wisdom);
	BOOST_CHECK_EQUAL(fix.actor.attributes[rpg::Attribute::Wisdom], 21u);
}

BOOST_AUTO_TEST_CASE(train_attribute_refreshs_properties) {
	auto& fix = Singleton<StatsFixture>::get();
	fix.reset();

	fix.actor.level = 5u;
	fix.actor.attributes[rpg::Attribute::Strength] = 40u;
	fix.actor.attributes[rpg::Attribute::Dexterity] = 20u;
	fix.actor.attributes[rpg::Attribute::Wisdom] = 10u;
	
	rpg::stats_impl::increaseAttribute(fix.context, fix.actor, rpg::Attribute::Wisdom);
	BOOST_CHECK_EQUAL(fix.actor.attributes[rpg::Attribute::Wisdom], 11u);
	BOOST_CHECK_EQUAL(fix.actor.base_props[rpg::Property::MaxLife], rpg::getMaxLife(fix.actor.attributes, fix.actor.level));
	BOOST_CHECK_EQUAL(fix.actor.base_props[rpg::Property::MaxMana], rpg::getMaxMana(fix.actor.attributes, fix.actor.level));
	BOOST_CHECK_EQUAL(fix.actor.base_props[rpg::Property::MaxStamina], rpg::getMaxStamina(fix.actor.attributes, fix.actor.level));
	BOOST_CHECK_EQUAL(fix.actor.base_props[rpg::Property::MagicBase], rpg::getMagicBase(fix.actor.attributes, fix.actor.level));
}

BOOST_AUTO_TEST_CASE(train_strength_refills_missing_stat_points) {
	auto& fix = Singleton<StatsFixture>::get();
	fix.reset();

	fix.actor.stats[rpg::Stat::Life] = fix.actor.properties[rpg::Property::MaxLife] - 3u;
	fix.actor.stats[rpg::Stat::Mana] = fix.actor.properties[rpg::Property::MaxMana] - 2u;
	fix.actor.stats[rpg::Stat::Stamina] = fix.actor.properties[rpg::Property::MaxStamina];
	
	rpg::stats_impl::increaseAttribute(fix.context, fix.actor, rpg::Attribute::Wisdom);
	
	BOOST_CHECK_EQUAL(fix.actor.properties[rpg::Property::MaxLife] - fix.actor.stats[rpg::Stat::Life], 3u);
	BOOST_CHECK_EQUAL(fix.actor.properties[rpg::Property::MaxMana] - fix.actor.stats[rpg::Stat::Mana], 2u);
	BOOST_CHECK_EQUAL(fix.actor.properties[rpg::Property::MaxStamina] - fix.actor.stats[rpg::Stat::Stamina], 0u);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(levelup_increases_level) {
	auto& fix = Singleton<StatsFixture>::get();
	fix.reset();

	rpg::stats_impl::onLevelup(fix.context, fix.actor);
	BOOST_CHECK_EQUAL(fix.actor.level, 41u);
}

BOOST_AUTO_TEST_CASE(levelup_refreshs_properties) {
	auto& fix = Singleton<StatsFixture>::get();
	fix.reset();

	fix.actor.level = 5u;
	fix.actor.attributes[rpg::Attribute::Strength] = 40u;
	fix.actor.attributes[rpg::Attribute::Dexterity] = 20u;
	fix.actor.attributes[rpg::Attribute::Wisdom] = 10u;
	
	rpg::stats_impl::onLevelup(fix.context, fix.actor);
	BOOST_CHECK_EQUAL(fix.actor.base_props[rpg::Property::MaxLife], rpg::getMaxLife(fix.actor.attributes, fix.actor.level));
	BOOST_CHECK_EQUAL(fix.actor.base_props[rpg::Property::MaxMana], rpg::getMaxMana(fix.actor.attributes, fix.actor.level));
	BOOST_CHECK_EQUAL(fix.actor.base_props[rpg::Property::MaxStamina], rpg::getMaxStamina(fix.actor.attributes, fix.actor.level));
	BOOST_CHECK_EQUAL(fix.actor.base_props[rpg::Property::MeleeBase], rpg::getMeleeBase(fix.actor.attributes, fix.actor.level));
	BOOST_CHECK_EQUAL(fix.actor.base_props[rpg::Property::RangeBase], rpg::getRangeBase(fix.actor.attributes, fix.actor.level));
	BOOST_CHECK_EQUAL(fix.actor.base_props[rpg::Property::MagicBase], rpg::getMagicBase(fix.actor.attributes, fix.actor.level));
}

BOOST_AUTO_TEST_CASE(levelup_restores_full_stats) {
	auto& fix = Singleton<StatsFixture>::get();
	fix.reset();

	fix.actor.level = 5u;
	fix.actor.stats[rpg::Stat::Life] = 1u;
	fix.actor.stats[rpg::Stat::Mana] = 1u;
	fix.actor.stats[rpg::Stat::Stamina] = 1u;
	fix.actor.attributes[rpg::Attribute::Strength] = 40u;
	fix.actor.attributes[rpg::Attribute::Dexterity] = 30u;
	fix.actor.attributes[rpg::Attribute::Wisdom] = 10u;

	rpg::stats_impl::onLevelup(fix.context, fix.actor);
	BOOST_CHECK_EQUAL(fix.actor.stats[rpg::Stat::Life], fix.actor.properties[rpg::Property::MaxLife]);
	BOOST_CHECK_EQUAL(fix.actor.stats[rpg::Stat::Mana], fix.actor.properties[rpg::Property::MaxMana]);
	BOOST_CHECK_EQUAL(fix.actor.stats[rpg::Stat::Stamina], fix.actor.properties[rpg::Property::MaxStamina]);
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(factor_only_affects_max_stats) {
	auto& fix = Singleton<StatsFixture>::get();
	fix.reset();

	std::vector<rpg::Property> max_stats;
	max_stats.push_back(rpg::Property::MaxLife);
	max_stats.push_back(rpg::Property::MaxMana);
	max_stats.push_back(rpg::Property::MaxStamina);
	auto properties = fix.actor.properties;
	fix.actor.factor = 0.5f;
	rpg::stats_impl::refresh(fix.actor);
	
	for (auto const & pair: fix.actor.properties) {
		if (utils::contains(max_stats, pair.first)) {
			BOOST_CHECK_EQUAL(pair.second, (std::uint32_t)std::ceil(properties[pair.first] * fix.actor.factor));
		} else {
			BOOST_CHECK_EQUAL(pair.second, properties[pair.first]);
		}
	}
}

BOOST_AUTO_TEST_SUITE_END()

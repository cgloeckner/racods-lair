#include <vector>
#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>
#include <testsuite/singleton.hpp>

#include <game/powerup.hpp>

struct PowerupFixture {
	core::IdManager id_manager;
	std::vector<core::ObjectID> ids;
	rpg::StatsManager stats;
	rpg::PlayerManager player;
	
	rpg::StatsSender stats_sender;
	game::PowerupSender powerup_sender;
	game::ReleaseListener release_listener;
	
	PowerupFixture()
		: id_manager{}
		, ids{}
		, stats{}
		, player{}
		, stats_sender{}
		, powerup_sender{}
		, release_listener{} {
	}
	
	void reset() {
		for (auto id : ids) {
			stats.release(id);
			if (player.has(id)) {
				player.release(id);
			}
		}
		ids.clear();
		stats.cleanup();
		player.cleanup();
		id_manager.reset();
		stats_sender.clear();
		powerup_sender.clear();
		release_listener.clear();
	}

	game::PowerupTrigger create(game::PowerupType type) {
		return game::PowerupTrigger{42u, stats, player, stats_sender, powerup_sender, release_listener, type};
	}

	core::ObjectID addObject(bool is_player) {
		auto id = id_manager.acquire();
		ids.push_back(id);
		auto& s = stats.acquire(id);
		s.stats[rpg::Stat::Life] = 1u;
		s.stats[rpg::Stat::Mana] = 1u;
		s.properties[rpg::Property::MaxLife] = 100;
		s.properties[rpg::Property::MaxMana] = 70;
		if (is_player) {
			player.acquire(id);
		}
		return id;
	}
};

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(powerup_test)

BOOST_AUTO_TEST_CASE(powerups_doesnt_work_for_non_players) {
	auto& fix = Singleton<PowerupFixture>::get();
	fix.reset();

	auto trigger = fix.create(game::PowerupType::Life);
	auto id = fix.addObject(false);
	trigger.execute(id);
	
	{
		auto const & events = fix.stats_sender.data();
		BOOST_REQUIRE(events.empty());
	}
	{
		auto const & events = fix.powerup_sender.data();
		BOOST_REQUIRE(events.empty());
	}
	{
		auto const & events = fix.release_listener.data();
		BOOST_REQUIRE(events.empty());
	}
	BOOST_CHECK(!trigger.isExpired());
}

BOOST_AUTO_TEST_CASE(powerups_ignores_dead_players) {
	auto& fix = Singleton<PowerupFixture>::get();
	fix.reset();

	auto trigger = fix.create(game::PowerupType::Life);
	auto id = fix.addObject(true);
	auto& s = fix.stats.query(id);
	s.stats[rpg::Stat::Life] = 0u;
	trigger.execute(id);
	
	{
		auto const & events = fix.stats_sender.data();
		BOOST_REQUIRE(events.empty());
	}
	{
		auto const & events = fix.powerup_sender.data();
		BOOST_REQUIRE(events.empty());
	}
	{
		auto const & events = fix.release_listener.data();
		BOOST_REQUIRE(events.empty());
	}
	BOOST_CHECK(!trigger.isExpired());
}

BOOST_AUTO_TEST_CASE(powerups_trigger_powerup_event_for_players) {
	auto& fix = Singleton<PowerupFixture>::get();
	fix.reset();

	auto trigger = fix.create(game::PowerupType::Life);
	auto id = fix.addObject(true);
	trigger.execute(id);
	
	auto const & events = fix.powerup_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, id);
}

BOOST_AUTO_TEST_CASE(powerups_trigger_stats_event_for_players) {
	auto& fix = Singleton<PowerupFixture>::get();
	fix.reset();

	auto trigger = fix.create(game::PowerupType::Life);
	auto id = fix.addObject(true);
	trigger.execute(id);
	
	auto const & events = fix.stats_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, id);
}

BOOST_AUTO_TEST_CASE(powerups_trigger_death_event_about_gem) {
	auto& fix = Singleton<PowerupFixture>::get();
	fix.reset();

	auto trigger = fix.create(game::PowerupType::Life);
	auto id = fix.addObject(true);
	trigger.execute(id);
	
	auto const & events = fix.release_listener.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, trigger.getId());
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(life_powerups_can_restore_half_life) {
	auto& fix = Singleton<PowerupFixture>::get();
	fix.reset();

	auto trigger = fix.create(game::PowerupType::Life);
	auto id = fix.addObject(true);
	
	auto& s = fix.stats.query(id);
	s.stats[rpg::Stat::Life] = 20;
	s.stats[rpg::Stat::Mana] = 10;
	trigger.execute(id);
	
	{
		auto const & events = fix.stats_sender.data();
		BOOST_REQUIRE_EQUAL(events.size(), 1u);
		BOOST_CHECK_EQUAL(events[0].actor, id);
		BOOST_CHECK_EQUAL(events[0].delta[rpg::Stat::Life], 50u);
		BOOST_CHECK_EQUAL(events[0].delta[rpg::Stat::Mana], 0u);
		BOOST_CHECK_EQUAL(events[0].delta[rpg::Stat::Stamina], 0u);
	}
	{
		auto const & events = fix.powerup_sender.data();
		BOOST_REQUIRE_EQUAL(events.size(), 1u);
		BOOST_CHECK_EQUAL(events[0].actor, id);
		BOOST_CHECK_EQUAL(events[0].delta[rpg::Stat::Life], 50u);
		BOOST_CHECK_EQUAL(events[0].delta[rpg::Stat::Mana], 0u);
		BOOST_CHECK_EQUAL(events[0].delta[rpg::Stat::Stamina], 0u);
	}
}

BOOST_AUTO_TEST_CASE(life_powerups_cannot_restore_life_if_fully_healed) {
	auto& fix = Singleton<PowerupFixture>::get();
	fix.reset();

	auto trigger = fix.create(game::PowerupType::Life);
	auto id = fix.addObject(true);
	
	auto& s = fix.stats.query(id);
	s.stats[rpg::Stat::Life] = 100;
	s.stats[rpg::Stat::Mana] = 10;
	trigger.execute(id);
	
	{
		auto const & events = fix.stats_sender.data();
		BOOST_REQUIRE(events.empty());
	}
	{
		auto const & events = fix.powerup_sender.data();
		BOOST_REQUIRE(events.empty());
	}
	BOOST_CHECK(!trigger.isExpired());
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(mana_powerups_can_restore_half_mana) {
	auto& fix = Singleton<PowerupFixture>::get();
	fix.reset();

	auto trigger = fix.create(game::PowerupType::Mana);
	auto id = fix.addObject(true);
	
	auto& s = fix.stats.query(id);
	s.stats[rpg::Stat::Life] = 20;
	s.stats[rpg::Stat::Mana] = 10;
	trigger.execute(id);
	
	{
		auto const & events = fix.stats_sender.data();
		BOOST_REQUIRE_EQUAL(events.size(), 1u);
		BOOST_CHECK_EQUAL(events[0].actor, id);
		BOOST_CHECK_EQUAL(events[0].delta[rpg::Stat::Life], 0u);
		BOOST_CHECK_EQUAL(events[0].delta[rpg::Stat::Mana], 35u);
		BOOST_CHECK_EQUAL(events[0].delta[rpg::Stat::Stamina], 0u);
	}
	{
		auto const & events = fix.powerup_sender.data();
		BOOST_REQUIRE_EQUAL(events.size(), 1u);
		BOOST_CHECK_EQUAL(events[0].actor, id);
		BOOST_CHECK_EQUAL(events[0].delta[rpg::Stat::Life], 0u);
		BOOST_CHECK_EQUAL(events[0].delta[rpg::Stat::Mana], 35u);
		BOOST_CHECK_EQUAL(events[0].delta[rpg::Stat::Stamina], 0u);
	}
}

BOOST_AUTO_TEST_CASE(mana_powerups_cannot_restore_mana_if_fully_healed) {
	auto& fix = Singleton<PowerupFixture>::get();
	fix.reset();

	auto trigger = fix.create(game::PowerupType::Mana);
	auto id = fix.addObject(true);
	
	auto& s = fix.stats.query(id);
	s.stats[rpg::Stat::Life] = 1;
	s.stats[rpg::Stat::Mana] = 70;
	trigger.execute(id);
	
	{
		auto const & events = fix.stats_sender.data();
		BOOST_REQUIRE(events.empty());
	}
	{
		auto const & events = fix.powerup_sender.data();
		BOOST_REQUIRE(events.empty());
	}
	BOOST_CHECK(!trigger.isExpired());
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(rejuv_powerups_can_restore_half_life_and_mana) {
	auto& fix = Singleton<PowerupFixture>::get();
	fix.reset();

	auto trigger = fix.create(game::PowerupType::Rejuvenation);
	auto id = fix.addObject(true);
	
	auto& s = fix.stats.query(id);
	s.stats[rpg::Stat::Life] = 20;
	s.stats[rpg::Stat::Mana] = 10;
	trigger.execute(id);
	
	{
		auto const & events = fix.stats_sender.data();
		BOOST_REQUIRE_EQUAL(events.size(), 1u);
		BOOST_CHECK_EQUAL(events[0].actor, id);
		BOOST_CHECK_EQUAL(events[0].delta[rpg::Stat::Life], 50u);
		BOOST_CHECK_EQUAL(events[0].delta[rpg::Stat::Mana], 35u);
		BOOST_CHECK_EQUAL(events[0].delta[rpg::Stat::Stamina], 0u);
	}
	{
		auto const & events = fix.powerup_sender.data();
		BOOST_REQUIRE_EQUAL(events.size(), 1u);
		BOOST_CHECK_EQUAL(events[0].actor, id);
		BOOST_CHECK_EQUAL(events[0].delta[rpg::Stat::Life], 50u);
		BOOST_CHECK_EQUAL(events[0].delta[rpg::Stat::Mana], 35u);
		BOOST_CHECK_EQUAL(events[0].delta[rpg::Stat::Stamina], 0u);
	}
}

BOOST_AUTO_TEST_CASE(rejuv_powerups_cannot_restore_life_and_mana_if_both_fully_healed) {
	auto& fix = Singleton<PowerupFixture>::get();
	fix.reset();

	auto trigger = fix.create(game::PowerupType::Rejuvenation);
	auto id = fix.addObject(true);
	
	auto& s = fix.stats.query(id);
	s.stats[rpg::Stat::Life] = 100;
	s.stats[rpg::Stat::Mana] = 70;
	trigger.execute(id);
	
	{
		auto const & events = fix.stats_sender.data();
		BOOST_REQUIRE(events.empty());
	}
	{
		auto const & events = fix.powerup_sender.data();
		BOOST_REQUIRE(events.empty());
	}
	BOOST_CHECK(!trigger.isExpired());
}

BOOST_AUTO_TEST_CASE(rejuv_powerups_can_restore_life_if_mana_is_fully_healed) {
	auto& fix = Singleton<PowerupFixture>::get();
	fix.reset();

	auto trigger = fix.create(game::PowerupType::Rejuvenation);
	auto id = fix.addObject(true);
	
	auto& s = fix.stats.query(id);
	s.stats[rpg::Stat::Life] = 20;
	s.stats[rpg::Stat::Mana] = 70;
	trigger.execute(id);
	
	{
		auto const & events = fix.stats_sender.data();
		BOOST_REQUIRE_EQUAL(events.size(), 1u);
		BOOST_CHECK_EQUAL(events[0].actor, id);
		BOOST_CHECK_EQUAL(events[0].delta[rpg::Stat::Life], 50u);
		BOOST_CHECK_EQUAL(events[0].delta[rpg::Stat::Mana], 35u);
		BOOST_CHECK_EQUAL(events[0].delta[rpg::Stat::Stamina], 0u);
	}
	{
		auto const & events = fix.powerup_sender.data();
		BOOST_REQUIRE_EQUAL(events.size(), 1u);
		BOOST_CHECK_EQUAL(events[0].actor, id);
		BOOST_CHECK_EQUAL(events[0].delta[rpg::Stat::Life], 50u);
		BOOST_CHECK_EQUAL(events[0].delta[rpg::Stat::Mana], 35u);
		BOOST_CHECK_EQUAL(events[0].delta[rpg::Stat::Stamina], 0u);
	}
}

BOOST_AUTO_TEST_CASE(rejuv_powerups_can_restore_mana_if_life_is_fully_healed) {
	auto& fix = Singleton<PowerupFixture>::get();
	fix.reset();

	auto trigger = fix.create(game::PowerupType::Rejuvenation);
	auto id = fix.addObject(true);
	
	auto& s = fix.stats.query(id);
	s.stats[rpg::Stat::Life] = 20;
	s.stats[rpg::Stat::Mana] = 70;
	trigger.execute(id);
	
	{
		auto const & events = fix.stats_sender.data();
		BOOST_REQUIRE_EQUAL(events.size(), 1u);
		BOOST_CHECK_EQUAL(events[0].actor, id);
		BOOST_CHECK_EQUAL(events[0].delta[rpg::Stat::Life], 50u);
		BOOST_CHECK_EQUAL(events[0].delta[rpg::Stat::Mana], 35u);
		BOOST_CHECK_EQUAL(events[0].delta[rpg::Stat::Stamina], 0u);
	}
	{
		auto const & events = fix.powerup_sender.data();
		BOOST_REQUIRE_EQUAL(events.size(), 1u);
		BOOST_CHECK_EQUAL(events[0].actor, id);
		BOOST_CHECK_EQUAL(events[0].delta[rpg::Stat::Life], 50u);
		BOOST_CHECK_EQUAL(events[0].delta[rpg::Stat::Mana], 35u);
		BOOST_CHECK_EQUAL(events[0].delta[rpg::Stat::Stamina], 0u);
	}
}

BOOST_AUTO_TEST_SUITE_END()

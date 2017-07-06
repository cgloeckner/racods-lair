#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>
#include <testsuite/singleton.hpp>

#include <rpg/balance.hpp>
#include <rpg/perk.hpp>

struct PerkFixture {
	core::LogContext log;
	core::AnimationSender animation_sender;
	rpg::QuickslotSender quickslot_sender;
	rpg::StatsSender stats_sender;
	rpg::PerkSender perk_sender;
	rpg::FeedbackSender feedback_sender;

	rpg::StatsManager stats;
	rpg::perk_impl::Context context;
	rpg::PerkData actor;

	rpg::PerkTemplate fireball;

	PerkFixture()
		: log{}
		, animation_sender{}
		, quickslot_sender{}
		, stats_sender{}
		, stats{}
		, context{log, animation_sender, quickslot_sender, stats_sender,
			  perk_sender, feedback_sender, stats}
		, actor{}
		, fireball{} {
		actor.id = 1u;
		stats.acquire(1u);
		
		fireball.damage[rpg::DamageType::Fire] = 0.8f;
		fireball.damage[rpg::DamageType::Magic] = 0.2f;
	}

	void reset() {
		actor.perks.clear();
		stats.query(1u).stats[rpg::Stat::Mana] = 100;

		animation_sender.clear();
		quickslot_sender.clear();
		stats_sender.clear();
		perk_sender.clear();
		feedback_sender.clear();
	}
};

BOOST_AUTO_TEST_SUITE(perk_test)

BOOST_AUTO_TEST_CASE(set_new_perks_level_creates_new_node) {
	auto& fix = Singleton<PerkFixture>::get();
	fix.reset();

	rpg::perk_impl::setPerkLevel(fix.context, fix.actor, fix.fireball, 2u);

	BOOST_REQUIRE_EQUAL(fix.actor.perks.size(), 1u);
	BOOST_CHECK(fix.actor.perks[0].perk == &fix.fireball);
	BOOST_CHECK_EQUAL(fix.actor.perks[0].level, 2u);
}

BOOST_AUTO_TEST_CASE(set_existing_perks_level_modifies_node) {
	auto& fix = Singleton<PerkFixture>::get();
	fix.reset();

	rpg::perk_impl::setPerkLevel(fix.context, fix.actor, fix.fireball, 2u);
	rpg::perk_impl::setPerkLevel(fix.context, fix.actor, fix.fireball, 4u);

	BOOST_REQUIRE_EQUAL(fix.actor.perks.size(), 1u);
	BOOST_CHECK(fix.actor.perks[0].perk == &fix.fireball);
	BOOST_CHECK_EQUAL(fix.actor.perks[0].level, 4u);
}

BOOST_AUTO_TEST_CASE(set_existing_perks_level_to_zero_deletes_node) {
	auto& fix = Singleton<PerkFixture>::get();
	fix.reset();

	rpg::perk_impl::setPerkLevel(fix.context, fix.actor, fix.fireball, 2u);
	rpg::perk_impl::setPerkLevel(fix.context, fix.actor, fix.fireball, 0u);

	BOOST_CHECK(fix.actor.perks.empty());
}

BOOST_AUTO_TEST_CASE(delete_perk_creates_quickslot_release_event) {
	auto& fix = Singleton<PerkFixture>::get();
	fix.reset();

	rpg::perk_impl::setPerkLevel(fix.context, fix.actor, fix.fireball, 2u);
	rpg::perk_impl::setPerkLevel(fix.context, fix.actor, fix.fireball, 0u);

	auto const& events = fix.quickslot_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, 1u);
	BOOST_CHECK_EQUAL(events[0].perk, &fix.fireball);
	BOOST_CHECK(events[0].type == rpg::QuickslotEvent::Release);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(cannot_calculate_mana_costs_if_perk_is_not_found) {
	auto& fix = Singleton<PerkFixture>::get();
	fix.reset();

	BOOST_CHECK_ASSERT(rpg::perk_impl::getManaCosts(fix.actor, fix.fireball));
}

BOOST_AUTO_TEST_CASE(mana_costs_depend_on_perk_level) {
	auto& fix = Singleton<PerkFixture>::get();
	fix.reset();

	rpg::perk_impl::setPerkLevel(fix.context, fix.actor, fix.fireball, 10u);
	auto costs = rpg::perk_impl::getManaCosts(fix.actor, fix.fireball);
	BOOST_CHECK_EQUAL(costs, rpg::getPerkCosts(1.f, 10u));
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(use_perk_without_enough_mana_will_just_trigger_idle) {
	auto& fix = Singleton<PerkFixture>::get();
	fix.reset();

	fix.stats.query(1u).stats[rpg::Stat::Mana] = 0;
	rpg::perk_impl::setPerkLevel(fix.context, fix.actor, fix.fireball, 7u);
	rpg::perk_impl::usePerk(fix.context, fix.actor, fix.fireball);

	BOOST_CHECK(fix.stats_sender.data().empty());
	BOOST_CHECK(fix.perk_sender.data().empty());
	auto const& events = fix.animation_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, 1u);
	BOOST_CHECK(events[0].type == core::AnimationEvent::Action);
	BOOST_CHECK(events[0].action == core::AnimationAction::Idle);
}

BOOST_AUTO_TEST_CASE(use_perk_without_enough_mana_sends_feedback) {
	auto& fix = Singleton<PerkFixture>::get();
	fix.reset();

	fix.stats.query(1u).stats[rpg::Stat::Mana] = 0;
	rpg::perk_impl::setPerkLevel(fix.context, fix.actor, fix.fireball, 7u);
	rpg::perk_impl::usePerk(fix.context, fix.actor, fix.fireball);

	BOOST_CHECK(fix.stats_sender.data().empty());
	BOOST_CHECK(fix.perk_sender.data().empty());

	auto const& events = fix.feedback_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, 1u);
	BOOST_CHECK(events[0].type == rpg::FeedbackType::NotEnoughMana);
}

BOOST_AUTO_TEST_CASE(use_perk_creates_stat_event_about_mana_consume) {
	auto& fix = Singleton<PerkFixture>::get();
	fix.reset();

	rpg::perk_impl::setPerkLevel(fix.context, fix.actor, fix.fireball, 7u);
	rpg::perk_impl::usePerk(fix.context, fix.actor, fix.fireball);

	auto expected = rpg::perk_impl::getManaCosts(fix.actor, fix.fireball);
	auto const& events = fix.stats_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, 1u);
	BOOST_CHECK_EQUAL(events[0].delta[rpg::Stat::Mana], -expected);
}

BOOST_AUTO_TEST_CASE(use_perk_forwards_perk_event) {
	auto& fix = Singleton<PerkFixture>::get();
	fix.reset();

	rpg::PerkEvent event;
	event.actor = 1u;
	event.perk = &fix.fireball;
	rpg::perk_impl::setPerkLevel(fix.context, fix.actor, fix.fireball, 7u);
	rpg::perk_impl::onUse(fix.context, fix.actor, event);

	auto const& events = fix.perk_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, 1u);
	BOOST_CHECK_EQUAL(events[0].perk, &fix.fireball);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(actor_hasnt_unlearned_perk) {
	auto& fix = Singleton<PerkFixture>::get();
	fix.reset();

	BOOST_CHECK(!rpg::hasPerk(fix.actor, fix.fireball));
}

BOOST_AUTO_TEST_CASE(actor_has_learned_perk) {
	auto& fix = Singleton<PerkFixture>::get();
	fix.reset();

	rpg::perk_impl::setPerkLevel(fix.context, fix.actor, fix.fireball, 2u);
	BOOST_CHECK(rpg::hasPerk(fix.actor, fix.fireball));
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(unlearned_perk_has_level_zero) {
	auto& fix = Singleton<PerkFixture>::get();
	fix.reset();

	auto level = rpg::getPerkLevel(fix.actor, fix.fireball);
	BOOST_CHECK_EQUAL(level, 0u);
}

BOOST_AUTO_TEST_CASE(learned_perk_has_valid_level) {
	auto& fix = Singleton<PerkFixture>::get();
	fix.reset();

	rpg::perk_impl::setPerkLevel(fix.context, fix.actor, fix.fireball, 2u);
	auto level = rpg::getPerkLevel(fix.actor, fix.fireball);
	BOOST_CHECK_EQUAL(level, 2u);

	rpg::perk_impl::setPerkLevel(fix.context, fix.actor, fix.fireball, 3u);
	level = rpg::getPerkLevel(fix.actor, fix.fireball);
	BOOST_CHECK_EQUAL(level, 3u);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(increase_perk_level_can_add_perk) {
	auto& fix = Singleton<PerkFixture>::get();
	fix.reset();

	rpg::perk_impl::onIncrease(fix.context, fix.actor, fix.fireball);
	auto level = rpg::getPerkLevel(fix.actor, fix.fireball);
	BOOST_CHECK_EQUAL(level, 1u);
}

BOOST_AUTO_TEST_CASE(increase_perk_level_can_increase_existing_perks_level) {
	auto& fix = Singleton<PerkFixture>::get();
	fix.reset();

	rpg::perk_impl::setPerkLevel(fix.context, fix.actor, fix.fireball, 3u);
	rpg::perk_impl::onIncrease(fix.context, fix.actor, fix.fireball);
	auto level = rpg::getPerkLevel(fix.actor, fix.fireball);
	BOOST_CHECK_EQUAL(level, 4u);
}

BOOST_AUTO_TEST_SUITE_END()

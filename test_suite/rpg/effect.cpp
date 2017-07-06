#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>
#include <testsuite/singleton.hpp>

#include <rpg/effect.hpp>

struct EffectFixture {
	core::LogContext log;
	rpg::BoniSender boni_sender;
	rpg::CombatSender combat_sender;
	rpg::EffectSender effect_sender;

	rpg::effect_impl::Context context;
	rpg::EffectData actor;

	rpg::EffectTemplate burn, poison, protect;

	EffectFixture()
		: log{}
		, boni_sender{}
		, combat_sender{}
		, effect_sender{}
		, context{log, boni_sender, combat_sender, effect_sender}
		, actor{}
		, burn{}
		, poison{}
		, protect{} {
		actor.id = 1u;

		burn.duration = sf::milliseconds(1500);
		poison.duration = sf::milliseconds(2000);
		protect.duration = sf::Time::Zero;  // perpetual effect
	}

	void reset() {
		actor.effects.clear();
		actor.cooldown = sf::Time::Zero;

		boni_sender.clear();
		combat_sender.clear();
		effect_sender.clear();
	}
};

BOOST_AUTO_TEST_SUITE(effect_test)

BOOST_AUTO_TEST_CASE(add_effect_creates_new_node) {
	auto& fix = Singleton<EffectFixture>::get();
	fix.reset();

	rpg::effect_impl::addEffect(fix.context, fix.actor, fix.burn);

	BOOST_REQUIRE_EQUAL(fix.actor.effects.size(), 1u);
	BOOST_CHECK_EQUAL(fix.actor.effects[0].effect, &fix.burn);
	BOOST_CHECK_TIME_EQUAL(fix.actor.effects[0].remain, fix.burn.duration);
}

BOOST_AUTO_TEST_CASE(add_effect_updates_existing_node) {
	auto& fix = Singleton<EffectFixture>::get();
	fix.reset();

	rpg::effect_impl::addEffect(fix.context, fix.actor, fix.burn);
	BOOST_REQUIRE_EQUAL(fix.actor.effects.size(), 1u);
	fix.actor.effects[0].remain = sf::milliseconds(250);

	rpg::effect_impl::addEffect(fix.context, fix.actor, fix.burn);
	BOOST_CHECK_TIME_EQUAL(fix.actor.effects[0].remain, fix.burn.duration);
}

BOOST_AUTO_TEST_CASE(add_effect_sends_boni_event) {
	auto& fix = Singleton<EffectFixture>::get();
	fix.reset();

	rpg::effect_impl::addEffect(fix.context, fix.actor, fix.burn);

	auto const& events = fix.boni_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, 1u);
	BOOST_CHECK_EQUAL(events[0].boni, &fix.burn.boni);
	BOOST_CHECK(events[0].type == rpg::BoniEvent::Add);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(remove_effect_removes_entire_node) {
	auto& fix = Singleton<EffectFixture>::get();
	fix.reset();

	rpg::effect_impl::addEffect(fix.context, fix.actor, fix.burn);
	rpg::effect_impl::removeEffect(fix.context, fix.actor, fix.burn);

	BOOST_CHECK(fix.actor.effects.empty());
}

BOOST_AUTO_TEST_CASE(remove_effect_sends_boni_event) {
	auto& fix = Singleton<EffectFixture>::get();
	fix.reset();

	rpg::effect_impl::addEffect(fix.context, fix.actor, fix.burn);
	fix.boni_sender.clear();
	rpg::effect_impl::removeEffect(fix.context, fix.actor, fix.burn);

	auto const& events = fix.boni_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, 1u);
	BOOST_CHECK_EQUAL(events[0].boni, &fix.burn.boni);
	BOOST_CHECK(events[0].type == rpg::BoniEvent::Remove);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(death_resets_entire_actor_state) {
	auto& fix = Singleton<EffectFixture>::get();
	fix.reset();

	rpg::effect_impl::addEffect(fix.context, fix.actor, fix.burn);
	fix.actor.cooldown = sf::milliseconds(150);

	rpg::effect_impl::onDeath(fix.context, fix.actor);
	BOOST_CHECK(fix.actor.effects.empty());
	BOOST_CHECK_TIME_EQUAL(fix.actor.cooldown, sf::Time::Zero);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(handle_effects_triggers_nothing_if_active_effects) {
	auto& fix = Singleton<EffectFixture>::get();
	fix.reset();

	rpg::effect_impl::handleEffects(fix.context, fix.actor, sf::Time::Zero);

	BOOST_CHECK(fix.context.combat_sender.data().empty());
}

BOOST_AUTO_TEST_CASE(handle_effects_triggers_combat_per_active_effect) {
	auto& fix = Singleton<EffectFixture>::get();
	fix.reset();

	rpg::effect_impl::addEffect(fix.context, fix.actor, fix.burn);
	rpg::effect_impl::addEffect(fix.context, fix.actor, fix.poison);

	rpg::effect_impl::handleEffects(fix.context, fix.actor, sf::Time::Zero);

	auto const& events = fix.context.combat_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 2u);
	BOOST_CHECK_EQUAL(events[0].target, 1u);
	BOOST_CHECK(events[0].meta_data.emitter == rpg::EmitterType::Effect);
	BOOST_CHECK_EQUAL(events[0].meta_data.effect, &fix.burn);
	BOOST_CHECK_EQUAL(events[1].target, 1u);
	BOOST_CHECK(events[1].meta_data.emitter == rpg::EmitterType::Effect);
	BOOST_CHECK_EQUAL(events[1].meta_data.effect, &fix.poison);
}

BOOST_AUTO_TEST_CASE(handle_effects_decreases_effects_remaining_time) {
	auto& fix = Singleton<EffectFixture>::get();
	fix.reset();

	rpg::effect_impl::addEffect(fix.context, fix.actor, fix.burn);
	rpg::effect_impl::addEffect(fix.context, fix.actor, fix.poison);

	rpg::effect_impl::handleEffects(
		fix.context, fix.actor, sf::milliseconds(500));

	BOOST_REQUIRE_EQUAL(fix.actor.effects.size(), 2u);
	BOOST_CHECK_TIME_EQUAL(fix.actor.effects[0].remain, sf::milliseconds(1000));
	BOOST_CHECK_TIME_EQUAL(fix.actor.effects[1].remain, sf::milliseconds(1500));
}

BOOST_AUTO_TEST_CASE(handle_effects_removes_finished_effects) {
	auto& fix = Singleton<EffectFixture>::get();
	fix.reset();

	rpg::effect_impl::addEffect(fix.context, fix.actor, fix.burn);
	rpg::effect_impl::addEffect(fix.context, fix.actor, fix.poison);

	rpg::effect_impl::handleEffects(
		fix.context, fix.actor, sf::milliseconds(1800));

	BOOST_REQUIRE_EQUAL(fix.actor.effects.size(), 1u);
	BOOST_CHECK_EQUAL(fix.actor.effects[0].effect, &fix.poison);
	BOOST_CHECK_TIME_EQUAL(fix.actor.effects[0].remain, sf::milliseconds(200));
}

BOOST_AUTO_TEST_CASE(handle_effects_will_remove_boni_if_effect_finished) {
	auto& fix = Singleton<EffectFixture>::get();
	fix.reset();

	rpg::effect_impl::addEffect(fix.context, fix.actor, fix.burn);
	rpg::effect_impl::addEffect(fix.context, fix.actor, fix.poison);
	fix.context.boni_sender.clear();
	rpg::effect_impl::handleEffects(
		fix.context, fix.actor, sf::milliseconds(1800));

	auto const& events = fix.context.boni_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, 1u);
	BOOST_CHECK_EQUAL(events[0].boni, &fix.burn.boni);
	BOOST_CHECK(events[0].type == rpg::BoniEvent::Remove);
}

BOOST_AUTO_TEST_CASE(handle_effects_will_propaget_removal_if_effect_finished) {
	auto& fix = Singleton<EffectFixture>::get();
	fix.reset();

	rpg::effect_impl::addEffect(fix.context, fix.actor, fix.burn);
	rpg::effect_impl::addEffect(fix.context, fix.actor, fix.poison);
	fix.context.boni_sender.clear();
	rpg::effect_impl::handleEffects(
		fix.context, fix.actor, sf::milliseconds(1800));

	auto const& events = fix.context.effect_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, 1u);
	BOOST_CHECK_EQUAL(events[0].effect, &fix.burn);
	BOOST_CHECK(events[0].type == rpg::EffectEvent::Remove);
}

BOOST_AUTO_TEST_CASE(handle_effects_keeps_perpetual_effects) {
	auto& fix = Singleton<EffectFixture>::get();
	fix.reset();

	rpg::effect_impl::addEffect(fix.context, fix.actor, fix.protect);
	fix.context.boni_sender.clear();
	rpg::effect_impl::handleEffects(
		fix.context, fix.actor, sf::milliseconds(1000));

	BOOST_CHECK(fix.context.boni_sender.data().empty());
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(update_will_sum_up_elapsed_time) {
	auto& fix = Singleton<EffectFixture>::get();
	fix.reset();

	fix.actor.cooldown = sf::milliseconds(250);
	rpg::effect_impl::onUpdate(fix.context, fix.actor, sf::milliseconds(100));

	BOOST_CHECK_TIME_EQUAL(fix.actor.cooldown, sf::milliseconds(350));
}

BOOST_AUTO_TEST_CASE(update_will_handle_effects_if_enough_time_went_by) {
	auto& fix = Singleton<EffectFixture>::get();
	fix.reset();

	rpg::effect_impl::addEffect(fix.context, fix.actor, fix.protect);
	rpg::effect_impl::onUpdate(fix.context, fix.actor, sf::milliseconds(rpg::effect_impl::MIN_ELAPSED_TIME));

	BOOST_CHECK(!fix.context.combat_sender.data().empty());
}

BOOST_AUTO_TEST_CASE(update_decreases_cooldown_if_effects_are_handled) {
	auto& fix = Singleton<EffectFixture>::get();
	fix.reset();

	fix.actor.cooldown = sf::milliseconds(300);
	rpg::effect_impl::addEffect(fix.context, fix.actor, fix.protect);
	rpg::effect_impl::onUpdate(fix.context, fix.actor, sf::milliseconds(rpg::effect_impl::MIN_ELAPSED_TIME + 200));

	BOOST_CHECK_TIME_EQUAL(fix.actor.cooldown, sf::milliseconds(500));
}

BOOST_AUTO_TEST_SUITE_END()

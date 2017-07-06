#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>
#include <testsuite/singleton.hpp>

#include <game/visuals.hpp>

struct VisualsFixture {

	core::LogContext log;
	core::RenderManager render_manager;
	core::AnimationSender animation_sender;
	game::visuals_impl::Context context;
	
	core::RenderData& obj;

	VisualsFixture()
		: log{}
		, render_manager{}
		, animation_sender{}
		, context{log, render_manager, animation_sender}
		, obj{render_manager.acquire(1u)} {
	}

	void reset() {
		obj = core::RenderData{};
		obj.id = 1u;
		
		animation_sender.clear();
	}
};

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(visuals_test)

BOOST_AUTO_TEST_CASE(death_causes_light_to_fade_out) {
	auto& fix = Singleton<VisualsFixture>::get();
	fix.reset();

	rpg::DeathEvent death;
	death.actor = 2u;
	game::visuals_impl::onKilled(fix.context, death);

	auto const& events = fix.context.animation_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 2u);
	BOOST_CHECK_EQUAL(events[0].actor, 2u);
	BOOST_CHECK(events[0].type == core::AnimationEvent::LightIntensity);
	BOOST_CHECK_CLOSE(events[0].interval.min, 0.0f, 0.0001f);
	BOOST_CHECK_CLOSE(events[0].interval.current, 1.f, 0.0001f);
	BOOST_CHECK_CLOSE(events[0].interval.max, 1.f, 0.0001f);
	BOOST_CHECK_CLOSE(
		events[0].interval.speed, 1.f / game::FADE_DELAY, 0.0001f);
	BOOST_CHECK(!events[0].interval.rise);
	BOOST_CHECK_EQUAL(events[0].interval.repeat, 1u);
}

BOOST_AUTO_TEST_CASE(death_causes_brightness_to_fade_out) {
	auto& fix = Singleton<VisualsFixture>::get();
	fix.reset();

	rpg::DeathEvent death;
	death.actor = 2u;
	game::visuals_impl::onKilled(fix.context, death);

	auto const& events = fix.context.animation_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 2u);
	BOOST_CHECK_EQUAL(events[1].actor, 2u);
	BOOST_CHECK(events[1].type == core::AnimationEvent::Brightness);
	BOOST_CHECK_CLOSE(events[1].interval.min, game::visuals_impl::BRIGHTNESS_ON_DEATH, 0.0001f);
	BOOST_CHECK_CLOSE(events[1].interval.current, 1.f, 0.0001f);
	BOOST_CHECK_CLOSE(events[1].interval.max, 1.f, 0.0001f);
	BOOST_CHECK_CLOSE(
		events[1].interval.speed, 0.7f / game::FADE_DELAY, 0.0001f);
	BOOST_CHECK(!events[1].interval.rise);
	BOOST_CHECK_EQUAL(events[1].interval.repeat, 1u);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(respawn_causes_light_to_fade_in) {
	auto& fix = Singleton<VisualsFixture>::get();
	fix.reset();

	rpg::SpawnEvent respawn;
	respawn.actor = fix.obj.id;
	game::visuals_impl::onSpawn(fix.context, respawn);

	auto const& events = fix.context.animation_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 2u);
	BOOST_CHECK_EQUAL(events[0].actor, fix.obj.id);
	BOOST_CHECK(events[0].type == core::AnimationEvent::LightIntensity);
	BOOST_CHECK_CLOSE(events[0].interval.min, 0.f, 0.0001f);
	BOOST_CHECK_CLOSE(events[0].interval.current, 0.f, 0.0001f);
	BOOST_CHECK_CLOSE(events[0].interval.max, 1.f, 0.0001f);
	BOOST_CHECK_CLOSE(
		events[0].interval.speed, 1.f / game::FADE_DELAY, 0.0001f);
	BOOST_CHECK(events[0].interval.rise);
	BOOST_CHECK_EQUAL(events[0].interval.repeat, 1u);
}

BOOST_AUTO_TEST_CASE(respawn_causes_brightness_to_fade_in) {
	auto& fix = Singleton<VisualsFixture>::get();
	fix.reset();

	rpg::SpawnEvent respawn;
	respawn.actor = fix.obj.id;
	game::visuals_impl::onSpawn(fix.context, respawn);

	auto const& events = fix.context.animation_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 2u);
	BOOST_CHECK_EQUAL(events[1].actor, fix.obj.id);
	BOOST_CHECK(events[1].type == core::AnimationEvent::Brightness);
	BOOST_CHECK_CLOSE(events[1].interval.min, game::visuals_impl::BRIGHTNESS_ON_DEATH, 0.0001f);
	BOOST_CHECK_CLOSE(events[1].interval.current, game::visuals_impl::BRIGHTNESS_ON_DEATH, 0.0001f);
	BOOST_CHECK_CLOSE(events[1].interval.max, 1.f, 0.0001f);
	BOOST_CHECK_CLOSE(
		events[1].interval.speed, 10.f / game::FADE_DELAY, 0.0001f);
	BOOST_CHECK(events[1].interval.rise);
	BOOST_CHECK_EQUAL(events[1].interval.repeat, 1u);
}

BOOST_AUTO_TEST_CASE(respawn_causes_light_animation_to_start) {
	auto& fix = Singleton<VisualsFixture>::get();
	fix.reset();

	fix.obj.light = std::make_unique<utils::Light>();
	fix.obj.light->radius = 123.f;
	rpg::SpawnEvent respawn;
	respawn.actor = fix.obj.id;
	game::visuals_impl::onSpawn(fix.context, respawn);

	auto const& events = fix.context.animation_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 3u);
	BOOST_CHECK_EQUAL(events[2].actor, fix.obj.id);
	BOOST_CHECK(events[2].type == core::AnimationEvent::LightRadius);
	BOOST_CHECK_EQUAL(events[2].interval.repeat, -1);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(bullet_explosion_causes_bullet_to_fade_away) {
	auto& fix = Singleton<VisualsFixture>::get();
	fix.reset();
	fix.obj.light = std::make_unique<utils::Light>();
	fix.obj.light->radius = 123.f;

	game::visuals_impl::onExploded(fix.context, 1u);

	auto const& events = fix.context.animation_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 2u);
	// bullet fade
	BOOST_CHECK_EQUAL(events[0].actor, 1u);
	BOOST_CHECK(events[0].type == core::AnimationEvent::Alpha);
	BOOST_CHECK_CLOSE(events[0].interval.current, 1.f, 0.0001f);
	BOOST_CHECK_CLOSE(events[0].interval.min, 0.f, 0.0001f);
	BOOST_CHECK_CLOSE(events[0].interval.max, 1.f, 0.0001f);
	BOOST_CHECK_CLOSE(events[0].interval.speed, 2.f / game::FADE_DELAY, 0.0001f);
	BOOST_CHECK(!events[0].interval.rise);
	BOOST_CHECK_EQUAL(events[0].interval.repeat, 1u);
	
	// light radius fade
	BOOST_CHECK_EQUAL(events[1].actor, 1u);
	BOOST_CHECK(events[1].type == core::AnimationEvent::LightRadius);
	BOOST_CHECK_CLOSE(events[1].interval.current, 123.f, 0.0001f);
	BOOST_CHECK_CLOSE(events[1].interval.min, 0.f, 0.0001f);
	BOOST_CHECK_CLOSE(events[1].interval.max, 123.f, 0.0001f);
	BOOST_CHECK_CLOSE(events[1].interval.speed, 123.f / game::FADE_DELAY, 0.0001f);
	BOOST_CHECK(!events[1].interval.rise);
	BOOST_CHECK_EQUAL(events[1].interval.repeat, 1u);
}


BOOST_AUTO_TEST_CASE(bullet_explosion_without_light_doesnt_trigger_light_animation) {
	auto& fix = Singleton<VisualsFixture>::get();
	fix.reset();

	game::visuals_impl::onExploded(fix.context, 1u);

	auto const& events = fix.context.animation_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	// bullet fade
	BOOST_CHECK_EQUAL(events[0].actor, 1u);
	BOOST_CHECK(events[0].type == core::AnimationEvent::Alpha);
	BOOST_CHECK_CLOSE(events[0].interval.current, 1.f, 0.0001f);
	BOOST_CHECK_CLOSE(events[0].interval.min, 0.f, 0.0001f);
	BOOST_CHECK_CLOSE(events[0].interval.max, 1.f, 0.0001f);
	BOOST_CHECK_CLOSE(events[0].interval.speed, 2.f / game::FADE_DELAY, 0.0001f);
	BOOST_CHECK(!events[0].interval.rise);
	BOOST_CHECK_EQUAL(events[0].interval.repeat, 1u);
}

BOOST_AUTO_TEST_SUITE_END()

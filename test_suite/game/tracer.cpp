#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>
#include <testsuite/singleton.hpp>

#include <utils/assert.hpp>
#include <game/tracer.hpp>

struct TracerPathDummy : game::PathSystem {
	TracerPathDummy(core::LogContext& log)
		: game::PathSystem{log} {
	}

	std::future<game::Path> schedule(core::ObjectID actor, utils::SceneID scene,
		sf::Vector2u const & source, sf::Vector2u const & target) override {
		game::Path path;
		auto current = sf::Vector2u{target};
		while (current.x > source.x) {
			path.push_back(current);
			current.x--;
		}
		while (current.x < source.x) {
			path.push_back(current);
			current.x++;
		}
		while (current.y > source.y) {
			path.push_back(current);
			current.y--;
		}
		while (current.y < source.y) {
			path.push_back(current);
			current.y++;
		}
		ASSERT(sf::Vector2u{current} == source);
		path.push_back(current);
		std::promise<game::Path> p;
		p.set_value(path);
		return p.get_future();
	}
};

struct TracerFixture {
	core::LogContext log;
	TracerPathDummy pathfinder;
	core::MovementManager movement;
	core::InputSender input_sender;
	game::TracerManager tracer;

	game::tracer_impl::Context context;
	game::TracerData& actor;

	TracerFixture()
		: log{}
		, pathfinder{log}
		, movement{}
		, input_sender{}
		, tracer{}
		, context{log, input_sender, movement, pathfinder}
		, actor{tracer.acquire(1u)} {
		movement.acquire(actor.id);
	}

	void reset() {
		actor.request = std::future<game::Path>{};
		actor.path.clear();
		actor.is_enabled = true;
		auto& mv = movement.query(actor.id);
		mv.pos = sf::Vector2f{1.5f, 1.5f};
		mv.last_pos = mv.pos;

		input_sender.clear();
		
		log.debug.clear();
		log.warning.clear();
		log.error.clear();
	}
};

BOOST_AUTO_TEST_SUITE(trace_test)

BOOST_AUTO_TEST_CASE(can_request_regular_path) {
	auto& fix = Singleton<TracerFixture>::get();
	fix.reset();
	
	auto& mv = fix.movement.query(fix.actor.id);
	
	// trigger path request
	game::tracer(fix.log, fix.context, mv, fix.actor, {3.f, 5.f});
	BOOST_REQUIRE(utils::isReady(fix.actor.request));
	BOOST_CHECK(fix.actor.path.empty());
	/// @note the dummy implementation returns a path immediately
	
	// trigger update, expect path and initial input event
	game::tracer_impl::onUpdate(fix.context, fix.actor);
	
	/// @note path is used backwards
	BOOST_REQUIRE_EQUAL(fix.actor.path.size(), 6u);
	BOOST_CHECK_VECTOR_EQUAL(fix.actor.path.back(), sf::Vector2u(1u, 2u));
	BOOST_CHECK_VECTOR_EQUAL(fix.actor.path.front(), sf::Vector2u(3u, 5u));
	
	auto const & events = fix.input_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, fix.actor.id);
	BOOST_CHECK_VECTOR_CLOSE(events[0].move, sf::Vector2f(0.f, 1.f), 0.0001f);
	fix.input_sender.clear();
	
	// reach 1st waypoint
	mv.pos = sf::Vector2f{1.5f, 2.49999f};
	mv.last_pos = sf::Vector2f{1.5f, 2.3f};
	game::tracer_impl::onUpdate(fix.context, fix.actor);
	BOOST_CHECK_VECTOR_EQUAL(fix.actor.path.back(), sf::Vector2u(1u, 3u));
	
	// reach 2nd waypoint be being too fast
	mv.pos = sf::Vector2f{1.5f, 3.666f};
	mv.last_pos = sf::Vector2f{1.5f, 3.43f};
	game::tracer_impl::onUpdate(fix.context, fix.actor);
	BOOST_CHECK_VECTOR_EQUAL(fix.actor.path.back(), sf::Vector2u(1u, 4u));
	
	// reach 3nd waypoint
	mv.pos = sf::Vector2f{1.5f, 4.500001f};
	mv.last_pos = sf::Vector2f{1.5f, 4.4f};
	game::tracer_impl::onUpdate(fix.context, fix.actor);
	BOOST_CHECK_VECTOR_EQUAL(fix.actor.path.back(), sf::Vector2u(1u, 5u));
	
	// reach 4th waypoint
	mv.pos = sf::Vector2f{1.5f, 5.500001f};
	mv.last_pos = sf::Vector2f{1.5f, 5.4f};
	game::tracer_impl::onUpdate(fix.context, fix.actor);
	BOOST_CHECK_VECTOR_EQUAL(fix.actor.path.back(), sf::Vector2u(2u, 5u));
	
	// expect movements "bottom", "bottom", "right"
	BOOST_REQUIRE_EQUAL(events.size(), 4u);
	BOOST_CHECK_VECTOR_CLOSE(events[0].move, sf::Vector2f(0.f, 1.f), 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(events[1].move, sf::Vector2f(0.f, 1.f), 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(events[2].move, sf::Vector2f(0.f, 1.f), 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(events[3].move, sf::Vector2f(1.f, 0.f), 0.0001f);
}

BOOST_AUTO_TEST_CASE(can_request_trivial_path) {
	auto& fix = Singleton<TracerFixture>::get();
	fix.reset();
	
	// trigger path request
	game::tracer(fix.log, fix.context, mv, fix.actor, {2.f, 1.f});
	BOOST_REQUIRE(utils::isReady(fix.actor.request));
	BOOST_CHECK(fix.actor.path.empty());
	/// @note the dummy implementation puts out a path immediately
	
	// trigger update, expect path and initial input event
	game::tracer_impl::onUpdate(fix.context, fix.actor);
	
	/// @note path is used backwards
	BOOST_REQUIRE_EQUAL(fix.actor.path.size(), 1u);
	BOOST_CHECK_VECTOR_EQUAL(fix.actor.path.back(), sf::Vector2u(2u, 1u));
	
	auto const & events = fix.input_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, fix.actor.id);
	BOOST_CHECK_VECTOR_CLOSE(events[0].move, sf::Vector2f(1.f, 0.f), 0.0001f);
}

BOOST_AUTO_TEST_CASE(can_request_empty_path) {
	auto& fix = Singleton<TracerFixture>::get();
	fix.reset();
	
	// trigger path request
	game::tracer(fix.log, fix.context, mv, fix.actor, {1.f, 1.f});
	BOOST_REQUIRE(utils::isReady(fix.actor.request));
	BOOST_CHECK(fix.actor.path.empty());
	/// @note the dummy implementation puts out a path immediately
	
	// trigger update, expect empty path and no input event
	game::tracer_impl::onUpdate(fix.context, fix.actor);
	BOOST_CHECK(fix.actor.path.empty());
	
	auto const & events = fix.input_sender.data();
	BOOST_REQUIRE(events.empty());
}

BOOST_AUTO_TEST_CASE(tracing_is_stopped_on_teleport) {
	auto& fix = Singleton<TracerFixture>::get();
	fix.reset();
	
	fix.actor.path.emplace_back(3.f, 2.f);
	fix.actor.path.emplace_back(2.f, 2.f);
	fix.actor.path.emplace_back(1.f, 2.f);
	
	BOOST_CHECK(fix.actor.is_enabled);
	game::tracer_impl::onTeleport(fix.actor);
	
	// expect actor's path be empty
	BOOST_CHECK(fix.actor.path.empty());
}

BOOST_AUTO_TEST_CASE(tracing_is_disabled_on_death) {
	auto& fix = Singleton<TracerFixture>::get();
	fix.reset();
	
	BOOST_CHECK(fix.actor.is_enabled);
	BOOST_CHECK(!utils::isReady(fix.actor.request));
	game::tracer_impl::onDeath(fix.actor);
	
	// expect actor be disabled and request be ignored
	BOOST_CHECK(!fix.actor.is_enabled);
	game::tracer(fix.log, fix.context, mv, fix.actor, {3.f, 5.f});
	BOOST_CHECK(!utils::isReady(fix.actor.request));
}

BOOST_AUTO_TEST_CASE(tracing_is_enabled_on_spawn) {
	auto& fix = Singleton<TracerFixture>::get();
	fix.reset();
	
	fix.actor.is_enabled = false;
	BOOST_CHECK(!utils::isReady(fix.actor.request));
	game::tracer_impl::onSpawn(fix.actor);
	
	// expect actor be enabled and request be processed
	BOOST_CHECK(fix.actor.is_enabled);
	game::tracer(fix.log, fix.context, mv, fix.actor, {3.f, 5.f});
	BOOST_CHECK(utils::isReady(fix.actor.request));
}

BOOST_AUTO_TEST_CASE(tracing_is_restarted_on_collision) {
	auto& fix = Singleton<TracerFixture>::get();
	fix.reset();
	
	fix.actor.path.emplace_back(3.f, 2.f);
	fix.actor.path.emplace_back(2.f, 2.f);
	fix.actor.path.emplace_back(1.f, 2.f);
	
	// expect proper path request after collision
	game::tracer_impl::onCollision(fix.context, fix.actor); 
	BOOST_REQUIRE(utils::isReady(fix.actor.request));
	BOOST_CHECK(fix.actor.path.empty());
	
	// trigger update, expect path and initial input event
	game::tracer_impl::onUpdate(fix.context, fix.actor);
	BOOST_REQUIRE_EQUAL(fix.actor.path.size(), 3u);
	BOOST_CHECK_VECTOR_CLOSE(fix.actor.path[0], sf::Vector2f(3.f, 2.f), 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(fix.actor.path[2], sf::Vector2f(1.f, 2.f), 0.0001f);
}

BOOST_AUTO_TEST_SUITE_END()

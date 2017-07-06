#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>
#include <testsuite/singleton.hpp>

#include <utils/assert.hpp>
#include <game/tracer.hpp>

/*
struct TracerPathDummy : game::PathSystem {
	TracerPathDummy()
		: game::PathSystem{} {
	}

	std::future<game::Path> schedule(game::PathPhase phase, core::ObjectID
actor, utils::SceneID scene, sf::Vector2u const & source, sf::Vector2u const &
target) override {
		game::Path path;
		auto current = target;
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
		ASSERT(current == source);
		path.push_back(current);
		std::promise<game::Path> p;
		p.set_value(path);
		return p.get_future();
	}
};

struct TracerFixture {
	core::LogContext log;
	TracerPathDummy pathfinder;
	core::MovementManager movement_manager;
	core::InputSender input_sender;

	game::PathTracer tracer;

	TracerFixture()
		: log{}
		, pathfinder{}
		, movement_manager{}
		, input_sender{}
		, tracer{log, pathfinder, movement_manager, input_sender, 1u} {
		movement_manager.acquire(1u);
	}

	void reset() {
		auto& data = movement_manager.query(1u);
		data = core::MovementData{};
		data.id = 1u;
		data.scene = 1u;

		tracer.reset();
		input_sender.clear();
	}
};

BOOST_AUTO_TEST_SUITE(trace_test)

BOOST_AUTO_TEST_CASE(empty_path_wont_throw_fail_event) {
	auto& fix = Singleton<TracerFixture>::get();
	fix.reset();

	fix.tracer.pathfind({0u, 0u});
	BOOST_CHECK(!fix.tracer.update()); // trigger
	BOOST_CHECK(!fix.tracer.update()); // wait
	BOOST_CHECK(fix.tracer.update()); // trace
	BOOST_CHECK(!fix.tracer.isRunning()); // trace
	BOOST_CHECK(fix.input_sender.data().empty());
}

BOOST_AUTO_TEST_CASE(finished_path_causes_idle) {
	auto& fix = Singleton<TracerFixture>::get();
	fix.reset();

	fix.tracer.pathfind({1u, 0u});
	fix.tracer.update(); // trigger
	fix.tracer.update(); // wait
	fix.tracer.update(); // trace
	auto const & events = fix.input_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, 1u);
	BOOST_CHECK_VECTOR_EQUAL(events[0].move, sf::Vector2i(1, 0));
}

BOOST_AUTO_TEST_CASE(uninitialized_path_causes_idle) {
	auto& fix = Singleton<TracerFixture>::get();
	fix.reset();

	BOOST_CHECK(!fix.tracer.isRunning());
	fix.tracer.update();
	BOOST_CHECK(!fix.tracer.isRunning());
	BOOST_CHECK(fix.input_sender.data().empty());
}

BOOST_AUTO_TEST_CASE(collision_causes_recalculation) {
	auto& fix = Singleton<TracerFixture>::get();
	fix.reset();

	fix.tracer.pathfind({2u, 3u});
	fix.tracer.update();
	BOOST_CHECK(fix.tracer.isRunning());
	fix.tracer.handle(core::CollisionEvent{});
	BOOST_CHECK(fix.tracer.isRunning());
}

BOOST_AUTO_TEST_CASE(path_is_recalculated_after_collision) {
	auto& fix = Singleton<TracerFixture>::get();
	fix.reset();

	fix.tracer.pathfind({2u, 3u});
	fix.tracer.update();
	fix.tracer.handle(core::CollisionEvent{});
	BOOST_CHECK(fix.tracer.isRunning());
	fix.tracer.update();
	BOOST_CHECK(fix.tracer.isRunning());
}

BOOST_AUTO_TEST_CASE(cannot_trace_next_waypoint_while_waypoint_is_not_reached) {
	auto& fix = Singleton<TracerFixture>::get();
	fix.reset();

	fix.tracer.pathfind({2u, 0u});
	fix.tracer.update(); // trigger
	fix.tracer.update(); // wait
	fix.tracer.update(); // trace
	auto const & events = fix.input_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	// update tracer without reaching waypoint
	for (auto i = 0u; i < 10; ++i) {
		fix.tracer.update();
	}
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	// reach waypoint and update tracer
	core::MoveEvent event;
	event.actor = 1u;
	event.type = core::MoveEvent::Left;
	event.target = sf::Vector2u{1u, 0u};
	fix.tracer.handle(event); // causes stop event!
	fix.tracer.update();
	BOOST_CHECK_EQUAL(events.size(), 3u);
}

BOOST_AUTO_TEST_CASE(calculated_path_causes_tracing) {
	auto& fix = Singleton<TracerFixture>::get();
	fix.reset();

	fix.tracer.pathfind({3u, 2u});
	fix.tracer.update(); // trigger
	fix.tracer.update(); // wait
	auto const & events = fix.input_sender.data();
	sf::Vector2u current;
	for (auto i = 0; i < 5; ++i) {
		BOOST_REQUIRE(!fix.tracer.update());
		auto const & last = events.back();
		current.x += last.move.x;
		current.y += last.move.y;
		core::MoveEvent event;
		event.actor = 1u;
		event.type = core::MoveEvent::Left;
		event.target = current;
		fix.tracer.handle(event);
	}
	// now finished
	BOOST_REQUIRE_EQUAL(events.size(), 10u);
	BOOST_CHECK_VECTOR_EQUAL(events[0].move, sf::Vector2u(0u, 1u));
	BOOST_CHECK_VECTOR_EQUAL(events[0].look, sf::Vector2u(0u, 1u));
	BOOST_CHECK_VECTOR_EQUAL(events[1].move, sf::Vector2u());
	BOOST_CHECK_VECTOR_EQUAL(events[2].move, sf::Vector2u(0u, 1u));
	BOOST_CHECK_VECTOR_EQUAL(events[2].look, sf::Vector2u(0u, 1u));
	BOOST_CHECK_VECTOR_EQUAL(events[3].move, sf::Vector2u());
	BOOST_CHECK_VECTOR_EQUAL(events[4].move, sf::Vector2u(1u, 0u));
	BOOST_CHECK_VECTOR_EQUAL(events[4].look, sf::Vector2u(1u, 0u));
	BOOST_CHECK_VECTOR_EQUAL(events[5].move, sf::Vector2u());
	BOOST_CHECK_VECTOR_EQUAL(events[6].move, sf::Vector2u(1u, 0u));
	BOOST_CHECK_VECTOR_EQUAL(events[6].look, sf::Vector2u(1u, 0u));
	BOOST_CHECK_VECTOR_EQUAL(events[7].move, sf::Vector2u());
	BOOST_CHECK_VECTOR_EQUAL(events[8].move, sf::Vector2u(1u, 0u));
	BOOST_CHECK_VECTOR_EQUAL(events[8].look, sf::Vector2u(1u, 0u));
	BOOST_CHECK_VECTOR_EQUAL(events[9].move, sf::Vector2u());
}

BOOST_AUTO_TEST_SUITE_END()
*/

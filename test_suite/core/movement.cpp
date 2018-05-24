#include <vector>
#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>
#include <testsuite/singleton.hpp>

#include <utils/assert.hpp>
#include <core/algorithm.hpp>
#include <core/movement.hpp>

struct MovementFixture {
	sf::Texture dummy_tileset;
	core::IdManager id_manager;
	std::vector<core::ObjectID> ids;

	core::LogContext log;
	core::MoveSender move_sender;
	core::MovementManager movement_manager;
	core::CollisionManager collision_manager;
	core::DungeonSystem dungeon_system;
	core::movement_impl::Context context;

	MovementFixture()
		: dummy_tileset{}
		, id_manager{}
		, log{}
		, move_sender{}
		, movement_manager{}
		, collision_manager{}
		, dungeon_system{}
		, context{log, move_sender, movement_manager, dungeon_system} {
		// add a scenes
		auto scene = dungeon_system.create(
			dummy_tileset, sf::Vector2u{12u, 10u}, sf::Vector2f{1.f, 1.f});
		assert(scene == 1u);
		auto& dungeon = dungeon_system[1u];
		for (auto y = 0u; y < 10u; ++y) {
			for (auto x = 0u; x < 12u; ++x) {
				if (x == 0u || x == 11u || y == 0u || y == 9u) {
					dungeon.getCell({x, y}).terrain = core::Terrain::Wall;
				} else {
					dungeon.getCell({x, y}).terrain = core::Terrain::Floor;
				}
			}
		}
	}

	void reset() {
		auto& dungeon = dungeon_system[1u];
		// clear dungeon
		for (auto y = 0u; y < 10u; ++y) {
			for (auto x = 0u; x < 12u; ++x) {
				dungeon.getCell({x, y}).entities.clear();
			}
		}
		// remove components
		for (auto id : ids) {
			movement_manager.release(id);
			collision_manager.release(id);
		}
		ids.clear();
		// cleanup systems
		id_manager.reset();
		movement_manager.cleanup();
		collision_manager.cleanup();
		// reset event senders
		move_sender.clear();
		
		// clear logs
		log.debug.clear();
		log.warning.clear();
		log.error.clear();
	}

	core::ObjectID add_object(sf::Vector2u const& pos, float max_speed) {
		auto id = id_manager.acquire();
		ids.push_back(id);
		auto& data = movement_manager.acquire(id);
		data.pos = sf::Vector2f{pos};
		data.target = pos;
		data.max_speed = max_speed;
		data.scene = 1u;
		auto& dungeon = dungeon_system[1u];
		dungeon.getCell(pos).entities.push_back(id);
		collision_manager.acquire(id);
		return id;
	}

	core::InputEvent move_object(core::ObjectID id, sf::Vector2i const& move) {
		core::InputEvent event;
		event.actor = id;
		event.move = move;
		event.look = move;
		return event;
	}

	void update(sf::Time const& elapsed) {
		core::updateChunked([&](sf::Time const& t) {
			core::movement_impl::updateRange(
				context, movement_manager.begin(), movement_manager.end(), t);
		}, elapsed, sf::milliseconds(core::MAX_FRAMETIME_MS));
	}
};

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(movement_test)

BOOST_AUTO_TEST_CASE(speed_mali_cause_small_speed_factor) {
	core::MovementData data;
	data.move = {1, 0};
	data.look = {1, 0};
	data.num_speed_boni = -5;
	auto factor = core::movement_impl::calcSpeedFactor(data);
	float expected =
		1.f + data.num_speed_boni * core::movement_impl::DELTA_SPEEDFACTOR;

	BOOST_CHECK_CLOSE(factor, expected, 0.0001f);
}

BOOST_AUTO_TEST_CASE(speed_boni_cause_large_speed_factor) {
	core::MovementData data;
	data.move = {1, 0};
	data.look = {1, 0};
	data.num_speed_boni = 5;
	auto factor = core::movement_impl::calcSpeedFactor(data);
	float expected =
		1.f + data.num_speed_boni * core::movement_impl::DELTA_SPEEDFACTOR;

	BOOST_CHECK_CLOSE(factor, expected, 0.0001f);
}

BOOST_AUTO_TEST_CASE(no_boni_or_mali_cause_default_speed_factor) {
	core::MovementData data;
	data.move = {1, 0};
	data.look = {1, 0};
	data.num_speed_boni = 0;
	auto factor = core::movement_impl::calcSpeedFactor(data);

	BOOST_CHECK_CLOSE(factor, 1.f, 0.0001f);
}

BOOST_AUTO_TEST_CASE(too_many_speed_mali_are_capped) {
	core::MovementData data;
	data.move = {1, 0};
	data.look = {1, 0};
	data.num_speed_boni = -21;
	auto factor = core::movement_impl::calcSpeedFactor(data);

	BOOST_CHECK_CLOSE(factor, core::movement_impl::MIN_SPEEDFACTOR, 0.0001f);
	BOOST_CHECK_GT(factor, 0.f);
}

BOOST_AUTO_TEST_CASE(too_many_speed_boni_are_capped) {
	core::MovementData data;
	data.move = {1, 0};
	data.look = {1, 0};
	data.num_speed_boni = 21;
	auto factor = core::movement_impl::calcSpeedFactor(data);

	BOOST_CHECK_CLOSE(factor, core::movement_impl::MAX_SPEEDFACTOR, 0.0001f);
}

BOOST_AUTO_TEST_CASE(moving_with_same_look_and_move_vector_is_forward) {
	core::MovementData data;
	data.move = {1, 0};
	data.look = {1, 0};
	BOOST_CHECK(core::movement_impl::getMoveStyle(data) == core::movement_impl::MoveStyle::Forward);
}

BOOST_AUTO_TEST_CASE(moving_with_look_move_delta_up_to_180_degree_is_sideward) {
	core::MovementData data;
	data.move = {1, 0};
	data.look = {0, -1};
	BOOST_CHECK(core::movement_impl::getMoveStyle(data) == core::movement_impl::MoveStyle::Sideward);
	data.look = {1, -1};
	BOOST_CHECK(core::movement_impl::getMoveStyle(data) == core::movement_impl::MoveStyle::Sideward);
	data.look = {1, 1};
	BOOST_CHECK(core::movement_impl::getMoveStyle(data) == core::movement_impl::MoveStyle::Sideward);
	data.look = {0, 1};
	BOOST_CHECK(core::movement_impl::getMoveStyle(data) == core::movement_impl::MoveStyle::Sideward);
}

BOOST_AUTO_TEST_CASE(moving_with_look_move_delta_above_180_degree_is_backward) {
	core::MovementData data;
	data.move = {1, 0};
	data.look = {-1, -1};
	BOOST_CHECK(core::movement_impl::getMoveStyle(data) == core::movement_impl::MoveStyle::Backward);
	data.look = {-1, 0};
	BOOST_CHECK(core::movement_impl::getMoveStyle(data) == core::movement_impl::MoveStyle::Backward);
	data.look = {-1, 1};
	BOOST_CHECK(core::movement_impl::getMoveStyle(data) == core::movement_impl::MoveStyle::Backward);
}

BOOST_AUTO_TEST_CASE(moving_backward_with_malus_results_in_low_factor) {
	core::MovementData data;
	data.move = {1, 0};
	data.look = {-1, -1};
	data.num_speed_boni = -1;
	auto factor = core::movement_impl::calcSpeedFactor(data);
	auto expect = (1.f - core::movement_impl::DELTA_SPEEDFACTOR) * core::movement_impl::BACKWARD_SPEEDFACTOR;
	
	BOOST_CHECK_CLOSE(factor, expect, 0.0001f);
}

BOOST_AUTO_TEST_CASE(moving_forward_causes_speedfactor_1) {
	core::MovementData data;
	data.move = {1, 0};
	data.look = {1, 0};
	auto factor = core::movement_impl::calcSpeedFactor(data);

	BOOST_CHECK_CLOSE(factor, 1.f, 0.0001f);
}

BOOST_AUTO_TEST_CASE(moving_sideward_causes_slightly_decreased_speedfactor) {
	core::MovementData data;
	data.move = {1, 0};
	data.look = {0, 1};
	auto factor = core::movement_impl::calcSpeedFactor(data);

	BOOST_CHECK_CLOSE(factor, core::movement_impl::SIDEWARD_SPEEDFACTOR, 0.0001f);
}

BOOST_AUTO_TEST_CASE(moving_backward_causes_decreased_speedfactor) {
	core::MovementData data;
	data.move = {1, 0};
	data.look = {-1, 0};
	auto factor = core::movement_impl::calcSpeedFactor(data);

	BOOST_CHECK_CLOSE(factor, core::movement_impl::BACKWARD_SPEEDFACTOR, 0.0001f);
}

BOOST_AUTO_TEST_CASE(cannot_interpolate_with_negative_speed) {
	auto& fix = Singleton<MovementFixture>::get();
	fix.reset();

	auto id = fix.add_object({5u, 1u}, -0.1f);
	auto& data = fix.movement_manager.query(id);

	// trigger movement
	auto event = fix.move_object(id, {-1, 1});
	core::movement_impl::start(fix.context, data, event);

	// trigger interpolation
	BOOST_CHECK_ASSERT(core::movement_impl::interpolate(
		fix.context, data, sf::milliseconds(50)));
}

BOOST_AUTO_TEST_CASE(cannot_interpolate_with_too_large_speed) {
	auto& fix = Singleton<MovementFixture>::get();
	fix.reset();

	auto id = fix.add_object({5u, 1u}, core::MAX_SPEED + 0.3f);
	auto& data = fix.movement_manager.query(id);

	// trigger movement
	auto event = fix.move_object(id, {-1, 1});
	core::movement_impl::start(fix.context, data, event);

	// trigger interpolation
	BOOST_CHECK_ASSERT(core::movement_impl::interpolate(
		fix.context, data, sf::milliseconds(50)));
}

BOOST_AUTO_TEST_CASE(object_remains_at_rest_without_move_vector) {
	auto& fix = Singleton<MovementFixture>::get();
	fix.reset();

	auto id = fix.add_object({5u, 1u}, 5.f);
	auto& data = fix.movement_manager.query(id);

	// try interpolation
	fix.update(sf::milliseconds(50));

	// assert old position
	BOOST_CHECK_VECTOR_CLOSE(data.pos, sf::Vector2f(5.f, 1.f), 0.0001f);
}

BOOST_AUTO_TEST_CASE(can_set_look_direction) {
	auto& fix = Singleton<MovementFixture>::get();
	fix.reset();

	auto id = fix.add_object({5u, 1u}, 5.f);
	auto& data = fix.movement_manager.query(id);

	// trigger movement
	core::InputEvent event;
	event.actor = id;
	event.look = {1, 0};
	core::movement_impl::start(fix.context, data, event);

	// assert new looking direction
	BOOST_CHECK_VECTOR_EQUAL(data.look, event.look);
	BOOST_CHECK(data.has_changed);
}

BOOST_AUTO_TEST_CASE(cannot_null_look_direction) {
	auto& fix = Singleton<MovementFixture>::get();
	fix.reset();

	auto id = fix.add_object({5u, 1u}, 5.f);
	auto& data = fix.movement_manager.query(id);
	data.has_changed = false;

	// trigger movement
	core::InputEvent event;
	event.actor = id;
	event.look = {0, 0};
	core::movement_impl::start(fix.context, data, event);

	// assert new looking direction
	BOOST_CHECK_VECTOR_EQUAL(data.look, sf::Vector2i(0, 1));
	BOOST_CHECK(!data.has_changed);
}

BOOST_AUTO_TEST_CASE(can_interpolate_common_movement) {
	auto& fix = Singleton<MovementFixture>::get();
	fix.reset();

	auto id = fix.add_object({5u, 1u}, 5.f);
	auto& data = fix.movement_manager.query(id);

	// trigger movement
	auto event = fix.move_object(id, {-1, 1});
	core::movement_impl::start(fix.context, data, event);

	// trigger interpolation
	fix.update(sf::milliseconds(50));

	// assert new position
	BOOST_CHECK_VECTOR_CLOSE(data.pos, sf::Vector2f(4.750f, 1.250f), 0.0001f);
}

BOOST_AUTO_TEST_CASE(can_interpolate_large_movement) {
	auto& fix = Singleton<MovementFixture>::get();
	fix.reset();

	auto id = fix.add_object({1u, 1u}, 5.f);
	auto& data = fix.movement_manager.query(id);

	// trigger movement
	auto event = fix.move_object(id, {1, 1});
	core::movement_impl::start(fix.context, data, event);

	// trigger interpolation
	fix.update(sf::seconds(5.f));

	// assert new position
	BOOST_CHECK_GT(data.pos.x, 3.f);
	BOOST_CHECK_GT(data.pos.y, 3.f);
}

BOOST_AUTO_TEST_CASE(movement_sets_dirtyflag) {
	auto& fix = Singleton<MovementFixture>::get();
	fix.reset();

	auto id = fix.add_object({5u, 1u}, 5.f);
	auto& data = fix.movement_manager.query(id);

	// trigger movement
	auto event = fix.move_object(id, {-1, 1});
	core::movement_impl::start(fix.context, data, event);

	// trigger interpolation
	fix.update(sf::milliseconds(50));

	// assert dirtyflag
	BOOST_CHECK(data.has_changed);
}

BOOST_AUTO_TEST_CASE(can_interpolate_movement_with_custom_factor) {
	auto& fix = Singleton<MovementFixture>::get();
	fix.reset();

	auto id = fix.add_object({5u, 1u}, 5.f);
	auto& data = fix.movement_manager.query(id);
	data.num_speed_boni = -8;

	// trigger movement
	auto event = fix.move_object(id, {-1, 1});
	core::movement_impl::start(fix.context, data, event);

	// trigger interpolation
	fix.update(sf::milliseconds(50));

	// assert new position
	BOOST_CHECK_VECTOR_CLOSE(data.pos, sf::Vector2f(4.850f, 1.150f), 0.0001f);
}

BOOST_AUTO_TEST_CASE(can_interpolate_over_multiple_tiles) {
	auto& fix = Singleton<MovementFixture>::get();
	fix.reset();

	auto id = fix.add_object({10u, 1u}, core::MAX_SPEED);
	auto& data = fix.movement_manager.query(id);

	// trigger movement
	auto event = fix.move_object(id, {-1, 1});
	core::movement_impl::start(fix.context, data, event);

	// trigger interpolation
	fix.update(sf::milliseconds(1000));

	// assert new position
	BOOST_CHECK_VECTOR_CLOSE(data.pos, sf::Vector2f(1.f, 10.f), 0.0001f);
}

/// @DEPRECATED
BOOST_AUTO_TEST_CASE(interpolate_over_multiple_tiles_triggers_multiple_events) {
	auto& fix = Singleton<MovementFixture>::get();
	fix.reset();

	auto id = fix.add_object({5u, 1u}, 5.f);
	auto& data = fix.movement_manager.query(id);

	// trigger movement
	auto event = fix.move_object(id, {-1, 1});
	core::movement_impl::start(fix.context, data, event);

	// trigger interpolation
	fix.update(sf::milliseconds(250));

	// assert multiple "tile left" and "tile reached" events
	auto const& moves = fix.move_sender.data();
	BOOST_REQUIRE_EQUAL(moves.size(), 3u);
	BOOST_CHECK_EQUAL(moves[0].actor, id);
	BOOST_CHECK_EQUAL(moves[0].type, core::MoveEvent::Left);
	BOOST_CHECK_VECTOR_EQUAL(moves[0].source, sf::Vector2u(5u, 1u));
	BOOST_CHECK_VECTOR_EQUAL(moves[0].target, sf::Vector2u(4u, 2u));
	BOOST_CHECK_EQUAL(moves[1].type, core::MoveEvent::Reached);
	BOOST_CHECK_VECTOR_EQUAL(moves[0].source, sf::Vector2u(5u, 1u));
	BOOST_CHECK_VECTOR_EQUAL(moves[0].target, sf::Vector2u(4u, 2u));
	BOOST_CHECK_EQUAL(moves[2].type, core::MoveEvent::Left);
	BOOST_CHECK_VECTOR_EQUAL(moves[2].source, sf::Vector2u(4u, 2u));
	BOOST_CHECK_VECTOR_EQUAL(moves[2].target, sf::Vector2u(3u, 3u));
	/*
	BOOST_CHECK_EQUAL(moves[3].type, core::MoveEvent::Reached);
	BOOST_CHECK_VECTOR_EQUAL(moves[3].source, sf::Vector2u(4u, 2u));
	BOOST_CHECK_VECTOR_EQUAL(moves[3].target, sf::Vector2u(3u, 3u));
	BOOST_CHECK_EQUAL(moves[4].type, core::MoveEvent::Left);
	BOOST_CHECK_VECTOR_EQUAL(moves[4].source, sf::Vector2u(3u, 3u));
	BOOST_CHECK_VECTOR_EQUAL(moves[4].target, sf::Vector2u(2u, 4u));
	BOOST_CHECK_EQUAL(moves[5].type, core::MoveEvent::Reached);
	BOOST_CHECK_VECTOR_EQUAL(moves[5].source, sf::Vector2u(3u, 3u));
	BOOST_CHECK_VECTOR_EQUAL(moves[5].target, sf::Vector2u(2u, 4u));
	*/
}

BOOST_AUTO_TEST_CASE(movement_can_be_stopped) {
	auto& fix = Singleton<MovementFixture>::get();
	fix.reset();

	auto id = fix.add_object({1u, 1u}, 5.f);
	auto& data = fix.movement_manager.query(id);

	// trigger movement
	auto event = fix.move_object(id, {1, 0});
	core::movement_impl::start(fix.context, data, event);

	// trigger interpolation
	fix.update(sf::milliseconds(1000));
	BOOST_REQUIRE_VECTOR_CLOSE(data.pos, sf::Vector2f(5.1f, 1.f), 0.0001f);

	// trigger idle
	event.move = {0, 0};
	core::movement_impl::start(fix.context, data, event);

	// try to continue interpolation
	fix.update(sf::milliseconds(1000));

	// assert position <4,1> where movement finished
	BOOST_CHECK_VECTOR_CLOSE(data.pos, sf::Vector2f(6.f, 1.f), 0.0001f);
}

BOOST_AUTO_TEST_CASE(movement_direction_can_be_modified) {
	auto& fix = Singleton<MovementFixture>::get();
	fix.reset();

	auto id = fix.add_object({5u, 1u}, 5.f);
	auto& data = fix.movement_manager.query(id);

	// trigger movement
	auto event = fix.move_object(id, {-1, 1});
	core::movement_impl::start(fix.context, data, event);

	// trigger interpolation
	fix.update(sf::milliseconds(3000));

	// trigger another direction
	event.move = {1, 0};
	event.look = {1, 0};
	core::movement_impl::start(fix.context, data, event);

	// try to continue interpolation
	fix.update(sf::milliseconds(2250));

	// assert new direction applied at position <3,3>
	// note: looking direction is changed while previous move direction is executed
	BOOST_CHECK_CLOSE(data.pos.y, 6.f, 0.0001f);
	BOOST_CHECK_GE(data.pos.x, 3.f);
}

BOOST_AUTO_TEST_CASE(movement_is_stopped_when_tile_is_reached) {
	auto& fix = Singleton<MovementFixture>::get();
	fix.reset();

	auto id = fix.add_object({5u, 1u}, 5.f);
	auto& data = fix.movement_manager.query(id);

	// trigger movement
	auto event = fix.move_object(id, {-1, 1});
	core::movement_impl::start(fix.context, data, event);
	/*
	auto const & moves = fix.move_sender.data();
	BOOST_REQUIRE_EQUAL(moves.size(), 1u);
	BOOST_CHECK_EQUAL(moves[0].actor, id);
	BOOST_CHECK_EQUAL(moves[0].type, core::MoveEvent::Left);
	*/

	// interpolate until tile was reached
	auto const& moves = fix.move_sender.data();
	int count = 0;
	while (true) {
		core::movement_impl::interpolate(
			fix.context, data, sf::milliseconds(100));
		if (moves.size() > 1u) {
			BOOST_REQUIRE_EQUAL(moves[1].type, core::MoveEvent::Reached);
			BOOST_CHECK_VECTOR_EQUAL(moves[1].target, sf::Vector2u(4.f, 2.f));
			break;
		}
		++count;
		if (count > 20) {
			BOOST_FAIL("Too many steps");
		}
	}

	// trigger idle
	event.move = {0, 0};
	core::movement_impl::start(fix.context, data, event);
	core::movement_impl::interpolate(fix.context, data, sf::milliseconds(50));

	// assert that object hasn't moved any further
	BOOST_CHECK_VECTOR_CLOSE(data.pos, sf::Vector2f(4.f, 2.f), 0.0001f);
}

BOOST_AUTO_TEST_CASE(
	object_movement_stopps_and_resets_position_as_collision_occures) {
	auto& fix = Singleton<MovementFixture>::get();
	fix.reset();

	auto id = fix.add_object({3u, 2u}, 15.f);
	auto& data = fix.movement_manager.query(id);
	data.pos = {3.f, 1.f};
	data.last_pos = {3.f, 2.f};

	// trigger movement
	auto event = fix.move_object(id, {0, -1});
	core::movement_impl::start(fix.context, data, event);

	// cause collision
	core::CollisionEvent ev;
	ev.actor = id;
	ev.interrupt = true;
	core::movement_impl::stop(fix.context, data, ev);

	// assert stop at position <3,2>
	BOOST_CHECK_VECTOR_CLOSE(data.pos, sf::Vector2f(3.f, 2.f), 0.0001f);
	auto& dungeon = fix.dungeon_system[1];
	auto& cell = dungeon.getCell({3u, 2u});
	BOOST_CHECK(utils::contains(cell.entities, id));
}

BOOST_AUTO_TEST_CASE(object_movement_is_continued_if_collision_does_not_interrupt) {
	auto& fix = Singleton<MovementFixture>::get();
	fix.reset();

	auto id = fix.add_object({1u, 1u}, 5.f);
	auto& data = fix.movement_manager.query(id);

	// trigger movement
	auto event = fix.move_object(id, {1, 0});
	core::movement_impl::start(fix.context, data, event);

	// assert moving
	BOOST_CHECK_VECTOR_EQUAL(data.next_move, sf::Vector2i(1, 0));
	
	// cause collision
	core::CollisionEvent ev;
	ev.actor = id;
	ev.interrupt = false;
	core::movement_impl::stop(fix.context, data, ev);
	
	// update using only small step because there is no collision system that
	// will update the collision grid as it is expected when leaving a tile
	fix.update(sf::milliseconds(10));

	// assert moving on
	BOOST_CHECK_VECTOR_EQUAL(data.move, sf::Vector2i(1, 0));
	BOOST_CHECK_LT(data.pos.y, 3.f);
}

BOOST_AUTO_TEST_CASE(bullet_movement_stopps_as_collision_occures) {
	auto& fix = Singleton<MovementFixture>::get();
	fix.reset();

	auto id = fix.add_object({3u, 2u}, 15.f);
	auto& data = fix.movement_manager.query(id);

	// trigger movement
	auto event = fix.move_object(id, {0, -1});
	core::movement_impl::start(fix.context, data, event);

	// interpolate until tile reached
	auto& moves = fix.move_sender.data();
	int steps = 0u;
	while (true) {
		fix.update(sf::milliseconds(20));
		if (moves.size() >= 2u) {
			BOOST_REQUIRE_EQUAL(moves[1].type, core::MoveEvent::Reached);
			BOOST_CHECK_EQUAL(moves[1].actor, id);
			BOOST_CHECK_VECTOR_EQUAL(moves[1].source, sf::Vector2u(3, 2));
			BOOST_CHECK_VECTOR_EQUAL(moves[1].target, sf::Vector2u(3, 1));
			break;
		}
		++steps;
		if (steps > 100) {
			BOOST_FAIL("Too many interpolation steps done");
		}
	}

	// cause collision
	core::CollisionEvent ev;
	ev.actor = id;
	ev.interrupt = true;
	// reset grid pos (is actually done by collision system)
	auto& dungeon = fix.dungeon_system[1];
	auto& src = dungeon.getCell({3u, 2u});
	auto& dst = dungeon.getCell({3u, 1u});
	BOOST_CHECK(utils::pop(src.entities, id));
	BOOST_CHECK(!utils::contains(dst.entities, id));
	dst.entities.push_back(id);
	data.pos = {3.f, 1.f};
	data.last_pos = {3.f, 2.f};

	// propagate event
	core::movement_impl::stop(fix.context, data, ev);

	// assert to be stopped at <3,2>
	BOOST_CHECK_VECTOR_CLOSE(data.pos, sf::Vector2f(3.f, 2.f), 0.0001f);
}

BOOST_AUTO_TEST_SUITE_END()

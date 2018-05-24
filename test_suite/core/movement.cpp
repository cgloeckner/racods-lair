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
		auto scene = dungeon_system.create(dummy_tileset, sf::Vector2u{12u, 10u},
			sf::Vector2f{1.f, 1.f});
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
		data.last_pos = data.pos;
		data.max_speed = max_speed;
		data.scene = 1u;
		data.look = {0.f, 1.f};
		auto& dungeon = dungeon_system[1u];
		dungeon.getCell(pos).entities.push_back(id);
		collision_manager.acquire(id);
		return id;
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

// ---------------------------------------------------------------------------
// Referring helper functions

BOOST_AUTO_TEST_CASE(moving_with_same_look_and_move_vector_is_forward) {
	core::MovementData data;
	data.move = {1.f, 0.f};
	data.look = {1.f, 0.f};
	BOOST_CHECK(core::movement_impl::getMoveStyle(data) == core::MoveStyle::Forward);
}

BOOST_AUTO_TEST_CASE(move_with_slightly_different_look_vector_is_forward) {
	core::MovementData data;
	data.move = {1.f, 0.f};
	data.look = thor::rotatedVector(data.move, 15.f);
	BOOST_CHECK(core::movement_impl::getMoveStyle(data) == core::MoveStyle::Forward);
}

BOOST_AUTO_TEST_CASE(moving_with_look_move_delta_up_to_180_degree_is_sideward) {
	core::MovementData data;
	data.move = {1.f, 0.f};
	data.look = {0.f, 1.f};
	BOOST_CHECK(core::movement_impl::getMoveStyle(data) == core::MoveStyle::Sideward);
	data.look = {1.f, -1.f};
	BOOST_CHECK(core::movement_impl::getMoveStyle(data) == core::MoveStyle::Sideward);
	data.look = {1.f, 1.f};
	BOOST_CHECK(core::movement_impl::getMoveStyle(data) == core::MoveStyle::Sideward);
	data.look = {0.f, 1.f};
	BOOST_CHECK(core::movement_impl::getMoveStyle(data) == core::MoveStyle::Sideward);
	data.look = thor::rotatedVector(data.move, 60.f);
	BOOST_CHECK(core::movement_impl::getMoveStyle(data) == core::MoveStyle::Sideward);
	data.look = thor::rotatedVector(data.move, 120.f);
	BOOST_CHECK(core::movement_impl::getMoveStyle(data) == core::MoveStyle::Sideward);
}

BOOST_AUTO_TEST_CASE(moving_with_look_move_delta_above_180_degree_is_backward) {
	core::MovementData data;
	data.move = {1.f, 0.f};
	data.look = {-1.f, -1.f};
	BOOST_CHECK(core::movement_impl::getMoveStyle(data) == core::MoveStyle::Backward);
	data.look = {-1.f, 0.f};
	BOOST_CHECK(core::movement_impl::getMoveStyle(data) == core::MoveStyle::Backward);
	data.look = {-1.f, 1.f};
	BOOST_CHECK(core::movement_impl::getMoveStyle(data) == core::MoveStyle::Backward);
	data.look = thor::rotatedVector(data.move, 140.f);
	BOOST_CHECK(core::movement_impl::getMoveStyle(data) == core::MoveStyle::Backward);
	data.look = thor::rotatedVector(data.move, 180.f);
	BOOST_CHECK(core::movement_impl::getMoveStyle(data) == core::MoveStyle::Backward);
}

BOOST_AUTO_TEST_CASE(speed_mali_cause_small_speed_factor) {
	core::MovementData data;
	data.move = {1.f, 0.f};
	data.look = {1.f, 0.f};
	data.num_speed_boni = -5;
	auto factor = core::movement_impl::calcSpeedFactor(data);
	float expected = 1.f + data.num_speed_boni * core::movement_impl::DELTA_SPEEDFACTOR;

	BOOST_CHECK_CLOSE(factor, expected, 0.0001f);
}

BOOST_AUTO_TEST_CASE(speed_boni_cause_large_speed_factor) {
	core::MovementData data;
	data.move = {1.f, 0.f};
	data.look = {1.f, 0.f};
	data.num_speed_boni = 5;
	auto factor = core::movement_impl::calcSpeedFactor(data);
	float expected = 1.f + data.num_speed_boni * core::movement_impl::DELTA_SPEEDFACTOR;

	BOOST_CHECK_CLOSE(factor, expected, 0.0001f);
}

BOOST_AUTO_TEST_CASE(no_boni_or_mali_cause_default_speed_factor) {
	core::MovementData data;
	data.move = {1.f, 0.f};
	data.look = {1.f, 0.f};
	data.num_speed_boni = 0;
	auto factor = core::movement_impl::calcSpeedFactor(data);

	BOOST_CHECK_CLOSE(factor, 1.f, 0.0001f);
}

BOOST_AUTO_TEST_CASE(too_many_speed_mali_are_capped) {
	core::MovementData data;
	data.move = {1.f, 0.f};
	data.look = {1.f, 0.f};
	data.num_speed_boni = -21;
	auto factor = core::movement_impl::calcSpeedFactor(data);

	BOOST_CHECK_CLOSE(factor, core::movement_impl::MIN_SPEEDFACTOR, 0.0001f);
	BOOST_CHECK_GT(factor, 0.f);
}

BOOST_AUTO_TEST_CASE(too_many_speed_boni_are_capped) {
	core::MovementData data;
	data.move = {1.f, 0.f};
	data.look = {1.f, 0.f};
	data.num_speed_boni = 21;
	auto factor = core::movement_impl::calcSpeedFactor(data);

	BOOST_CHECK_CLOSE(factor, core::movement_impl::MAX_SPEEDFACTOR, 0.0001f);
}

BOOST_AUTO_TEST_CASE(moving_backward_with_malus_results_in_low_factor) {
	core::MovementData data;
	data.move = {1.f, 0.f};
	data.look = {-1.f, -1.f};
	data.num_speed_boni = -1;
	auto factor = core::movement_impl::calcSpeedFactor(data);
	auto expect = (1.f - core::movement_impl::DELTA_SPEEDFACTOR) * core::movement_impl::BACKWARD_SPEEDFACTOR;
	
	BOOST_CHECK_CLOSE(factor, expect, 0.0001f);
}

BOOST_AUTO_TEST_CASE(moving_forward_causes_speedfactor_1) {
	core::MovementData data;
	data.move = {1.f, 0.f};
	data.look = {1.f, 0.f};
	auto factor = core::movement_impl::calcSpeedFactor(data);

	BOOST_CHECK_CLOSE(factor, 1.f, 0.0001f);
}

BOOST_AUTO_TEST_CASE(moving_sideward_causes_slightly_decreased_speedfactor) {
	core::MovementData data;
	data.move = {1.f, 0.f};
	data.look = {0.f, 1.f};
	auto factor = core::movement_impl::calcSpeedFactor(data);

	BOOST_CHECK_CLOSE(factor, core::movement_impl::SIDEWARD_SPEEDFACTOR, 0.0001f);
}

BOOST_AUTO_TEST_CASE(moving_backward_causes_decreased_speedfactor) {
	core::MovementData data;
	data.move = {1.f, 0.f};
	data.look = {-1.f, 0.f};
	auto factor = core::movement_impl::calcSpeedFactor(data);

	BOOST_CHECK_CLOSE(factor, core::movement_impl::BACKWARD_SPEEDFACTOR, 0.0001f);
}

BOOST_AUTO_TEST_CASE(cannot_interpolate_with_negative_speed) {
	auto& fix = Singleton<MovementFixture>::get();
	fix.reset();

	auto id = fix.add_object({5u, 1u}, -0.1f);
	auto& data = fix.movement_manager.query(id);

	// trigger movement
	core::movement_impl::setMovement(fix.context, data, {-1.f, 1.f}, {-1.f, 1.f});

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
	core::movement_impl::setMovement(fix.context, data, {-1.f, 1.f}, {-1.f, 1.f});

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

BOOST_AUTO_TEST_CASE(can_interpolate_small_movement) {
	auto& fix = Singleton<MovementFixture>::get();
	fix.reset();

	auto id = fix.add_object({5u, 1u}, 5.f);
	auto& data = fix.movement_manager.query(id);

	// trigger movement
	core::movement_impl::setMovement(fix.context, data, {-1.f, 1.f}, {-1.f, 1.f});

	// trigger interpolation
	fix.update(sf::milliseconds(50));

	// assert new position
	BOOST_CHECK_GT(data.pos.x, 4.5f);
	BOOST_CHECK_GT(data.pos.y, 1.f);
}

BOOST_AUTO_TEST_CASE(can_interpolate_large_movement) {
	auto& fix = Singleton<MovementFixture>::get();
	fix.reset();

	auto id = fix.add_object({1u, 1u}, 5.f);
	auto& data = fix.movement_manager.query(id);

	// trigger movement
	core::movement_impl::setMovement(fix.context, data, {1.f, 1.f}, {1.f, 1.f});

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
	core::movement_impl::setMovement(fix.context, data, {-1.f, 1.f}, {-1.f, 1.f});

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
	core::movement_impl::setMovement(fix.context, data, {-1.f, 1.f}, {-1.f, 1.f});

	// trigger interpolation
	fix.update(sf::milliseconds(50));

	// assert new position
	BOOST_CHECK_GT(data.pos.x, 4.5f);
	BOOST_CHECK_GT(data.pos.y, 1.f);
}

BOOST_AUTO_TEST_CASE(movement_can_be_stopped) {
	auto& fix = Singleton<MovementFixture>::get();
	fix.reset();

	auto id = fix.add_object({1u, 1u}, 5.f);
	auto& data = fix.movement_manager.query(id);

	// move and then step
	core::movement_impl::setMovement(fix.context, data, {1.f, 0.f}, {1.f, 0.f});
	fix.update(sf::milliseconds(1000));
	auto pos = data.pos;
	core::movement_impl::setMovement(fix.context, data, {}, data.look);

	// try to continue interpolation
	fix.update(sf::milliseconds(1000));

	// expect previous position
	BOOST_CHECK_VECTOR_CLOSE(data.pos, pos, 0.0001f);
}

BOOST_AUTO_TEST_CASE(movement_direction_can_be_modified) {
	auto& fix = Singleton<MovementFixture>::get();
	fix.reset();

	auto id = fix.add_object({5u, 1u}, 5.f);
	auto& data = fix.movement_manager.query(id);

	// trigger movement
	core::movement_impl::setMovement(fix.context, data, {-1.f, 1.f}, {-1.f, 1.f});

	// trigger interpolation
	fix.update(sf::milliseconds(3000));

	// trigger another direction
	core::movement_impl::setMovement(fix.context, data, {1.f, 0.f}, {1.f, 0.f});

	// try to continue interpolation
	fix.update(sf::milliseconds(2250));

	// assert new direction applied at position <3,3>
	// note: looking direction is changed while previous move direction is executed
	BOOST_CHECK_GT(data.pos.y, 10.f);
	BOOST_CHECK_GT(data.pos.x, 4.f);
}

BOOST_AUTO_TEST_CASE(object_movement_stopps_movement_as_interrupt_collision_occures) {
	auto& fix = Singleton<MovementFixture>::get();
	fix.reset();

	auto id = fix.add_object({3u, 2u}, 15.f);
	auto& data = fix.movement_manager.query(id);
	data.pos = {3.f, 1.f};
	data.last_pos = {3.f, 2.f};

	// trigger movement
	core::movement_impl::setMovement(fix.context, data, {0.f, -1.f}, {0.f, -1.f});

	// cause collision
	core::CollisionEvent ev;
	ev.actor = id;
	ev.interrupt = true;
	core::movement_impl::onCollision(fix.context, data, ev);

	// assert stop at (invalid) position <3,1>
	/// @note that the position reset and collision map update are handled by the
	///		collision system (not the movement system)
	BOOST_CHECK_VECTOR_CLOSE(data.pos, sf::Vector2f(3.f, 1.f), 0.0001f);
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
	core::movement_impl::setMovement(fix.context, data, {1.f, 0.f}, {1.f, 0.f});

	// assert moving
	BOOST_CHECK_VECTOR_EQUAL(data.move, sf::Vector2f(1.f, 0.f));
	
	// cause collision
	core::CollisionEvent ev;
	ev.actor = id;
	ev.interrupt = false;
	core::movement_impl::onCollision(fix.context, data, ev);
	
	// update using only small step because there is no collision system that
	// will update the collision grid as it is expected when leaving a tile
	fix.update(sf::milliseconds(10));

	// assert moving on
	BOOST_CHECK_VECTOR_EQUAL(data.move, sf::Vector2f(1.f, 0.f));
	BOOST_CHECK_LT(data.pos.y, 3.f);
}

/// @TODO test MoveEvent::Start & ::Stop

BOOST_AUTO_TEST_SUITE_END()

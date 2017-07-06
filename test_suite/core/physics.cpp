#include <vector>
#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>
#include <testsuite/singleton.hpp>

#include <utils/assert.hpp>
#include <core/algorithm.hpp>
#include <core/physics.hpp>

/*
struct DemoTrigger: core::BaseTrigger {
	bool flag;
	
	DemoTrigger()
		: core::BaseTrigger{}
		, flag{false} {
	}
	
	void execute(core::ObjectID actor) override {
		flag = true;
	}
	
	bool isExpired() const override {
		return flag;
	}
};
*/

struct PhysicsFixture {
	sf::Texture dummy_tileset;
	core::IdManager id_manager;
	std::vector<core::ObjectID> ids;
	
	core::CollisionSender collision_sender;
	core::FocusSender focus_sender;

	core::LogContext log;
	core::PhysicsManager physics_manager;
	core::DungeonSystem dungeon_system;
	core::physics_impl::Context context;

	PhysicsFixture()
		: dummy_tileset{}
		, id_manager{}
		, collision_sender{}
		, focus_sender{}
		, log{}
		, physics_manager{}
		, dungeon_system{}
		, context{log, collision_sender, focus_sender, physics_manager, dungeon_system} {
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
			physics_manager.release(id);
		}
		ids.clear();
		// cleanup systems
		id_manager.reset();
		physics_manager.cleanup();
		// reset collision events
		collision_sender.clear();
		focus_sender.clear();
	}

	core::ObjectID add_object(sf::Vector2u const& pos, float max_speed, bool is_bullet=false) {
		auto id = id_manager.acquire();
		ids.push_back(id);
		auto& data = physics_manager.acquire(id);
		data.pos = sf::Vector2f{pos};
		data.max_speed = max_speed;
		data.scene = 1u;
		data.is_projectile = is_bullet;
		if (!is_bullet) {
			data.radius = 0.5f;
		} else{
			data.radius = 0.15f;
		}
		auto& dungeon = dungeon_system[1u];
		dungeon.getCell(pos).entities.push_back(id);
		return id;
	}

	core::InputEvent move_object(core::ObjectID id, sf::Vector2f const& move) {
		core::InputEvent event;
		event.actor = id;
		event.move = move;
		event.look = move;
		return event;
	}

	core::InputEvent face_object(core::ObjectID id, sf::Vector2f const& look) {
		core::InputEvent event;
		event.actor = id;
		event.move = sf::Vector2f{};
		event.look = look;
		return event;
	}

	void update(sf::Time const& elapsed) {
		core::updateChunked([&](sf::Time const& t) {
			core::physics_impl::updateRange(
				context, physics_manager.begin(), physics_manager.end(), t);
		}, elapsed, sf::milliseconds(core::MAX_FRAMETIME_MS));
	}
};

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(physics_test)

// --------------------------------------------------------------------
// --- Physics

BOOST_AUTO_TEST_CASE(movement_is_interpolated) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();
	
	// create actor
	auto id = fix.add_object({1u, 1u}, 5.f);
	auto& data = fix.physics_manager.query(id);

	// trigger movement
	auto event = fix.move_object(id, {0.6f, 0.8f});
	core::physics_impl::start(fix.context, data, event);

	// trigger simulation
	fix.update(sf::seconds(1.f));
	
	// expect position
	auto expect = data.pos + data.move * data.max_speed * core::physics_impl::MOVEMENT_VELOCITY;
	BOOST_CHECK_VECTOR_CLOSE(data.pos, expect, 0.001f);
}

BOOST_AUTO_TEST_CASE(movement_updates_collision_map) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();
	
	// create actor
	auto id = fix.add_object({1u, 1u}, 5.f);
	auto& data = fix.physics_manager.query(id);

	// trigger movement
	auto event = fix.move_object(id, {0.6f, 0.8f});
	core::physics_impl::start(fix.context, data, event);

	// trigger simulation
	fix.update(sf::seconds(5.f));
	
	// expect position
	auto const & src = fix.dungeon_system[1u].getCell({1u, 1u});
	auto const & dst = fix.dungeon_system[1u].getCell(sf::Vector2u(data.pos));
	BOOST_CHECK(!utils::contains(src.entities, id));
	BOOST_CHECK(utils::contains(dst.entities, id));
}

BOOST_AUTO_TEST_CASE(trigger_is_activated_as_tile_is_reached) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();
	
	// tba
}

// --------------------------------------------------------------------
// --- Collision

BOOST_AUTO_TEST_CASE(objects_can_collide_with_each_other) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();
	
	// create actor
	auto id = fix.add_object({1u, 1u}, 5.f);
	auto& data = fix.physics_manager.query(id);

	// create bullet
	auto id2 = fix.add_object({3u, 3u}, 5.f);
	auto& data2 = fix.physics_manager.query(id2);

	// trigger movements
	auto event = fix.move_object(id, {0.7f, 0.7f});
	core::physics_impl::start(fix.context, data, event);
	
	auto event2 = fix.move_object(id2, {-0.7f, -0.7f});
	core::physics_impl::start(fix.context, data2, event2);

	// trigger simulation
	fix.update(sf::seconds(1.f));
	
	auto const & events = fix.collision_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, id2);
	BOOST_CHECK_EQUAL(events[0].collider, id);
	BOOST_CHECK_VECTOR_CLOSE(data.pos, events[0].pos, 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(data.pos, sf::Vector2f(), 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(data2.pos, sf::Vector2f(), 0.0001f);
}

BOOST_AUTO_TEST_CASE(bullet_can_collide_with_regular_objects) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();
	
	// create actor
	auto id = fix.add_object({1u, 1u}, 5.f);
	auto& data = fix.physics_manager.query(id);

	// create bullet
	auto id2 = fix.add_object({3u, 3u}, 8.f, true);
	auto& data2 = fix.physics_manager.query(id2);

	// trigger movements
	auto event = fix.move_object(id, {0.7f, 0.7f});
	core::physics_impl::start(fix.context, data, event);
	
	auto event2 = fix.move_object(id2, {-0.7f, -0.7f});
	core::physics_impl::start(fix.context, data2, event2);

	// trigger simulation
	fix.update(sf::seconds(1.f));
	
	auto const & events = fix.collision_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, id2);
	BOOST_CHECK_EQUAL(events[0].collider, id);
	BOOST_CHECK_VECTOR_CLOSE(data.pos, events[0].pos, 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(data.pos, sf::Vector2f(), 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(data2.pos, sf::Vector2f(), 0.0001f);
}

BOOST_AUTO_TEST_CASE(bullet_does_not_collide_with_allied_object) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();
	
	// create actor
	auto id = fix.add_object({1u, 1u}, 5.f);
	auto& data = fix.physics_manager.query(id);

	// create bullet
	auto id2 = fix.add_object({3u, 3u}, 8.f, true);
	auto& data2 = fix.physics_manager.query(id2);
	data2.ignore.push_back(id);

	// trigger movements
	auto event = fix.move_object(id, {0.7f, 0.7f});
	core::physics_impl::start(fix.context, data, event);
	
	auto event2 = fix.move_object(id2, {-0.7f, -0.7f});
	core::physics_impl::start(fix.context, data2, event2);

	// trigger simulation
	fix.update(sf::seconds(1.f));
	
	auto const & events = fix.collision_sender.data();
	BOOST_CHECK(events.empty());
}

BOOST_AUTO_TEST_CASE(bullets_do_not_collide_with_each_other) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();
	
	// create actor
	auto id = fix.add_object({1u, 1u}, 10.f, true);
	auto& data = fix.physics_manager.query(id);

	// create bullet
	auto id2 = fix.add_object({3u, 3u}, 8.f, true);
	auto& data2 = fix.physics_manager.query(id2);

	// trigger movements
	auto event = fix.move_object(id, {0.7f, 0.7f});
	core::physics_impl::start(fix.context, data, event);
	
	auto event2 = fix.move_object(id2, {-0.7f, -0.7f});
	core::physics_impl::start(fix.context, data2, event2);

	// trigger simulation
	fix.update(sf::seconds(1.f));
	
	auto const & events = fix.collision_sender.data();
	BOOST_CHECK(events.empty());
}

BOOST_AUTO_TEST_CASE(objects_can_collide_with_terrain) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();
	
	// create actor
	auto id = fix.add_object({1u, 1u}, 5.f);
	auto& data = fix.physics_manager.query(id);

	// trigger movement
	auto event = fix.move_object(id, {-0.7f, -0.7f});
	core::physics_impl::start(fix.context, data, event);
	
	// trigger simulation
	fix.update(sf::seconds(1.f));
	
	auto const & events = fix.collision_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, id);
	BOOST_CHECK_EQUAL(events[0].collider, 0);
	BOOST_CHECK_VECTOR_CLOSE(data.pos, events[0].pos, 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(data.pos, sf::Vector2f(), 0.0001f);
}

// --------------------------------------------------------------------
// --- Focus

/*

BOOST_AUTO_TEST_CASE(focus_update_picks_closest_object) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();
	
	// create actor
	auto id = fix.add_object({1u, 1u}, 5.f);
	auto& data = fix.physics_manager.query(id);
	
	// create target
	auto id2 = fix.add_object({2u, 1u}, 5.f);
	auto& data2 = fix.physics_manager.query(id2);
	
	// create another object
	auto id3 = fix.add_object({2u, 2u}, 5.f);
	
	auto focus_id = core::queryFocus(fix.context, data);
	
	BOOST_CHECK_EQUAL(focus_id, id2);
}

BOOST_AUTO_TEST_CASE(focus_is_updated_as_look_is_changed) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();
	
	// create actor
	auto id = fix.add_object({1u, 1u}, 5.f);
	auto& data = fix.physics_manager.query(id);
	
	// create target
	auto id2 = fix.add_object({2u, 1u}, 5.f);
	auto& data2 = fix.physics_manager.query(id2);
	
	auto event = fix.face_object(id, {-0.7f, -0.7f});
}

BOOST_AUTO_TEST_CASE(focus_is_updated_as_pos_is_changed) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();
	
}

*/

BOOST_AUTO_TEST_SUITE_END()

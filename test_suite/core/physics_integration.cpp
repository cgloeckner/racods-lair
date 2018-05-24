#include <vector>
#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>
#include <testsuite/singleton.hpp>

#include <utils/assert.hpp>
#include <utils/algorithm.hpp>
#include <core/algorithm.hpp>
#include <core/collision.hpp>
#include <core/dungeon.hpp>
#include <core/focus.hpp>
#include <core/movement.hpp>
#include <core/teleport.hpp>

struct PhysicsFixture
	: utils::EventListener<core::CollisionEvent, core::MoveEvent,
		core::FocusEvent, core::TeleportEvent> {
	sf::Texture dummy_tileset;
	core::IdManager id_manager;
	core::LogContext log;
	std::vector<core::ObjectID> ids;

	std::vector<core::CollisionEvent> collisions;
	std::vector<core::MoveEvent> moves;
	std::vector<core::FocusEvent> focuses;
	std::vector<core::TeleportEvent> teleports;

	core::DungeonSystem dungeon;
	core::CollisionSystem collision;
	core::MovementSystem movement;
	core::FocusSystem focus;

	utils::SceneID scene;

	PhysicsFixture()
		: utils::EventListener<core::CollisionEvent, core::MoveEvent,
			  core::FocusEvent, core::TeleportEvent>{}
		, dummy_tileset{}
		, id_manager{}
		, log{}
		, ids{}
		, collisions{}
		, moves{}
		, focuses{}
		, teleports{}
		, dungeon{}
		, collision{log, 1000u, dungeon, movement}
		, movement{log, 1000u, dungeon}
		, focus{log, 1000u} {
		// connect events
		collision.bind<core::CollisionEvent>(movement);
		collision.bind<core::CollisionEvent>(*this);
		collision.bind<core::TeleportEvent>(*this);
		movement.bind<core::MoveEvent>(*this);

		// add scenes
		sf::Vector2u grid_size{10u, 10u};
		scene =
			dungeon.create(dummy_tileset, grid_size, sf::Vector2f{1.f, 1.f});
		auto& d = dungeon[scene];
		for (auto y = 0u; y < grid_size.y; ++y) {
			for (auto x = 0u; x < grid_size.x; ++x) {
				auto& cell = d.getCell({x, y});
				if (x == 0u || x == grid_size.x - 1u || y == 0u ||
					y == grid_size.y - 1u) {
					cell.terrain = core::Terrain::Wall;
				} else {
					cell.terrain = core::Terrain::Floor;
				}
			}
		}
	}

	void reset() {
		auto& d = dungeon[scene];
		// clear dungeon
		for (auto y = 0u; y < 10u; ++y) {
			for (auto x = 0u; x < 10u; ++x) {
				auto& cell = d.getCell({x, y});
				cell.entities.clear();
				cell.trigger = nullptr;
			}
		}
		// remove components
		for (auto id : ids) {
			movement.release(id);
			collision.release(id);
			focus.release(id);
		}
		ids.clear();
		// cleanup systems
		id_manager.reset();
		movement.cleanup();
		collision.cleanup();
		focus.cleanup();
		// reset event stuff
		dynamic_cast<core::CollisionListener&>(movement).clear();
		dynamic_cast<core::MoveListener&>(focus).clear();
		dynamic_cast<core::MoveListener&>(collision).clear();
		collisions.clear();
		moves.clear();
		focuses.clear();
		teleports.clear();
		
		// clear logs
		log.debug.clear();
		log.warning.clear();
		log.error.clear();
	}

	void handle(core::CollisionEvent const& event) {
		collisions.push_back(event);
	}
	void handle(core::MoveEvent const& event) { moves.push_back(event); }
	void handle(core::FocusEvent const& event) { focuses.push_back(event); }
	void handle(core::TeleportEvent const& event) { teleports.push_back(event); }

	void addTeleport(utils::SceneID from, sf::Vector2u const& at,
		utils::SceneID to, sf::Vector2u const& dst) {
		auto& trigger = dungeon[from].getCell(at).trigger;
		trigger = std::make_unique<core::TeleportTrigger>(
			dynamic_cast<core::TeleportSender&>(collision), movement,
			collision, dungeon, to, dst);
	}

	core::ObjectID add_object(utils::SceneID scene, sf::Vector2u const& pos,
		sf::Vector2i const& look, float sight, float max_speed) {
		auto id = id_manager.acquire();
		ids.push_back(id);
		auto& f = focus.acquire(id);
		f.sight = sight;
		if (f.sight > 0.f) {
			f.display_name = "foo";
		}
		auto& m = movement.acquire(id);
		m.pos = sf::Vector2f{pos};
		m.last_pos = m.pos;
		m.scene = scene;
		m.max_speed = max_speed;
		m.look = sf::Vector2f{look};
		auto&c = collision.acquire(id);
		c.shape.radius = 0.5f;
		auto& d = dungeon[scene];
		d.getCell(pos).entities.push_back(id);
		return id;
	}

	core::ObjectID add_bullet(utils::SceneID scene, sf::Vector2f const& pos,
		sf::Vector2f const& look, float sight, float max_speed) {
		auto id = id_manager.acquire();
		ids.push_back(id);
		auto& f = focus.acquire(id);
		f.sight = sight;
		auto& m = movement.acquire(id);
		m.pos = pos;
		m.scene = scene;
		m.max_speed = max_speed;
		m.look = look;
		auto& c = collision.acquire(id);
		c.is_projectile = true;
		c.shape.radius = 0.2f;
		auto& d = dungeon[scene];
		d.getCell(sf::Vector2u{pos}).entities.push_back(id);
		move_object(id, look, look);
		return id;
	}

	void move_object(core::ObjectID id, sf::Vector2f const& move, sf::Vector2f const& look) {
		core::InputEvent event;
		event.actor = id;
		event.move = move;
		event.look = look;
		movement.receive(event);
	}

	void rotate_object(core::ObjectID id, sf::Vector2f const& look) {
		core::InputEvent event;
		event.actor = id;
		event.move = movement.query(id).move; // reuse previous move vector
		event.look = look;
		movement.receive(event);
	}

	void update(sf::Time const& elapsed) {
		core::updateChunked([&](sf::Time const& t) {
			movement.update(t);
			collision.update(t);
			focus.update(t);
		}, elapsed, sf::milliseconds(core::MAX_FRAMETIME_MS));

		dispatch<core::CollisionEvent>(*this);
		dispatch<core::MoveEvent>(*this);
		dispatch<core::FocusEvent>(*this);
		dispatch<core::TeleportEvent>(*this);
	}
};

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(physics_integration)

// ---------------------------------------------------------------------------
// --- COLLISION TESTS

BOOST_AUTO_TEST_CASE(bullet_can_collide_with_regular_objects) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();

	auto entity = fix.add_object(fix.scene, {3u, 2u}, {0, 1}, 5.f, 5.f);
	auto bullet = fix.add_bullet(fix.scene, {5u, 2u}, {-1, 0}, 1.f, 5.f);

	fix.update(sf::seconds(0.5f));
	
	// expect object collision between bullet and entity
	auto const& colls = fix.collisions;
	BOOST_REQUIRE_EQUAL(colls.size(), 1u);
	BOOST_CHECK_EQUAL(colls[0].actor, bullet);
	BOOST_CHECK_EQUAL(colls[0].collider, entity);

	// expect bullet moving on!
	auto const& e_m = fix.movement.query(entity);
	auto const& b_m = fix.movement.query(bullet);
	BOOST_CHECK_VECTOR_CLOSE(b_m.move, sf::Vector2f(-1.f, 0.f), 0.0001f);
	BOOST_CHECK_LT(b_m.pos.x, e_m.pos.x);
}

BOOST_AUTO_TEST_CASE(bullet_stops_movement_if_it_hits_an_unaccessable_tile) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();

	auto bullet = fix.add_bullet(fix.scene, {4u, 2u}, {-1, 0}, 1.f, 10.f);
	
	fix.update(sf::seconds(10.f));

	// expect tile collision
	auto const& colls = fix.collisions;
	BOOST_REQUIRE_EQUAL(colls.size(), 1u);
	BOOST_CHECK_EQUAL(colls[0].actor, bullet);
	BOOST_CHECK_EQUAL(colls[0].collider, 0u);

	// expect bullet has stopped at (1.3, 2)
	auto const& b_m = fix.movement.query(bullet);
	BOOST_CHECK_VECTOR_CLOSE(b_m.move, sf::Vector2f(), 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(b_m.pos, sf::Vector2f(1.3f, 2.f), 0.0001f);
}

BOOST_AUTO_TEST_CASE(
	object_stops_movement_if_it_collides_with_standing_object) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();

	auto mover = fix.add_object(fix.scene, {3u, 2u}, {-1, 0}, 1.f, 5.f);
	auto idler = fix.add_object(fix.scene, {3u, 4u}, {-1, 0}, 1.f, 5.f);
	fix.move_object(mover, {0.f, 1.f}, {0.f, 1.f});
	
	fix.update(sf::seconds(8.f));

	// expect object collision
	auto const& colls = fix.collisions;
	BOOST_REQUIRE_EQUAL(colls.size(), 1u);
	BOOST_CHECK_EQUAL(colls[0].actor, mover);
	BOOST_CHECK_EQUAL(colls[0].collider, idler);

	// expect mover has stopped near (3, 3)
	auto const& m_m = fix.movement.query(mover);
	BOOST_CHECK_VECTOR_EQUAL(m_m.move, sf::Vector2i());
	BOOST_CHECK_VECTOR_CLOSE(m_m.pos, sf::Vector2f(3.f, 2.875f), 0.001f);
}

BOOST_AUTO_TEST_CASE(
	object_stops_movement_if_it_collides_directly_neighbored_object) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();

	auto mover = fix.add_object(fix.scene, {3u, 2u}, {-1, 0}, 1.f, 5.f);
	auto idler = fix.add_object(fix.scene, {3u, 3u}, {-1, 0}, 1.f, 5.f);
	fix.move_object(mover, {0.f, 1.f}, {0.f, 1.f});
	fix.update(sf::seconds(8.f));

	// expect object collision
	auto const& colls = fix.collisions;
	BOOST_REQUIRE_EQUAL(colls.size(), 1u);
	BOOST_CHECK_EQUAL(colls[0].actor, mover);
	BOOST_CHECK_EQUAL(colls[0].collider, idler);

	// expect mover has stopped at (3, 2)
	auto const& m_m = fix.movement.query(mover);
	BOOST_CHECK_VECTOR_EQUAL(m_m.move, sf::Vector2i());
	BOOST_CHECK_VECTOR_CLOSE(m_m.pos, sf::Vector2f(3.f, 2.f), 0.0001f);
}

BOOST_AUTO_TEST_CASE(
	object_stops_movement_if_it_collides_directly_neighbored_unaccessable_tile) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();

	auto object = fix.add_object(fix.scene, {3u, 1u}, {-1, 0}, 1.f, 5.f);
	fix.move_object(object, {0.f, -1.f}, {0.f, 1.f});
	fix.update(sf::seconds(8.f));

	// expect object collision
	auto const& colls = fix.collisions;
	BOOST_REQUIRE_EQUAL(colls.size(), 1u);
	BOOST_CHECK_EQUAL(colls[0].actor, object);
	BOOST_CHECK_EQUAL(colls[0].collider, 0u);

	// expect mover has stopped at (3, 1)
	auto const& o_m = fix.movement.query(object);
	BOOST_CHECK_VECTOR_EQUAL(o_m.move, sf::Vector2i());
	BOOST_CHECK_VECTOR_CLOSE(o_m.pos, sf::Vector2f(3.f, 1.f), 0.0001f);
}

BOOST_AUTO_TEST_CASE(
	object_stops_movement_if_it_collides_with_a_crossing_object) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();

	auto cross = fix.add_object(fix.scene, {2u, 4u}, {-1, 0}, 1.f, 5.f);
	auto mover = fix.add_object(fix.scene, {4u, 2u}, {-1, 0}, 1.f, 5.f);
	fix.move_object(cross, {1.f, 0.f}, {1.f, 0.f});
	fix.move_object(mover, {0.f, 1.f}, {0.f, 1.f});
	fix.update(sf::seconds(6.f));

	// expect actor to collide with crossing object
	auto const& colls = fix.collisions;
	BOOST_REQUIRE_EQUAL(colls.size(), 2u);
	BOOST_CHECK_EQUAL(colls[0].actor, cross);
	BOOST_CHECK_EQUAL(colls[0].collider, mover);
	BOOST_CHECK_EQUAL(colls[1].actor, mover);
	BOOST_CHECK_EQUAL(colls[1].collider, cross);

	// expect actor object has stopped near (4, 3)
	auto const& m_m = fix.movement.query(mover);
	BOOST_CHECK_VECTOR_EQUAL(m_m.move, sf::Vector2i());
	BOOST_CHECK_VECTOR_CLOSE(m_m.pos, sf::Vector2f(4.f, 3.175f), 0.001f);

	// expect crossing object stopped, too
	auto const& c_m = fix.movement.query(cross);
	BOOST_CHECK_VECTOR_EQUAL(c_m.move, sf::Vector2i());
	BOOST_CHECK_VECTOR_CLOSE(c_m.pos, sf::Vector2f(3.175f, 4.f), 0.001f);
	
	// check distance
	auto const & m_c = fix.collision.query(mover);
	auto const & c_c = fix.collision.query(cross);
	auto dist = utils::distance(m_m.pos, c_m.pos); // is squared
	auto radsum = m_c.shape.radius + c_c.shape.radius;
	BOOST_CHECK_LE(radsum * radsum, dist);
}

BOOST_AUTO_TEST_CASE(direct_tunnelingtest) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();

	auto mover = fix.add_object(fix.scene, {1u, 2u}, {-1, 0}, 1.f, core::MAX_SPEED);
	auto oncom = fix.add_object(fix.scene, {5u, 2u}, {-1, 0}, 1.f, core::MAX_SPEED);
	fix.move_object(mover, {1.f, 0.f}, {1.f, 0.f});
	fix.move_object(oncom, {-1.f, 0.f}, {1.f, 0.f});
	fix.update(sf::seconds(8.f));

	// expect collisions
	auto const& colls = fix.collisions;
	BOOST_REQUIRE_EQUAL(colls.size(), 2u);
	BOOST_CHECK_EQUAL(colls[0].actor, oncom);
	BOOST_CHECK_EQUAL(colls[0].collider, mover);
	BOOST_CHECK_EQUAL(colls[1].actor, mover);
	BOOST_CHECK_EQUAL(colls[1].collider, oncom);

	// expect actor has stopped near (3, 2)
	auto const& m_m = fix.movement.query(mover);
	BOOST_CHECK_VECTOR_EQUAL(m_m.move, sf::Vector2i());
	BOOST_CHECK_VECTOR_CLOSE(m_m.pos, sf::Vector2f(2.f, 2.f), 0.001f);
	
	// and expect oncomming object has stopped too
	auto const& o_m = fix.movement.query(oncom);
	BOOST_CHECK_VECTOR_EQUAL(o_m.move, sf::Vector2i());
	BOOST_CHECK_VECTOR_CLOSE(o_m.pos, sf::Vector2f(5.f, 2.f), 0.001f);
	
	// check distance
	auto const & m_c = fix.collision.query(mover);
	auto const & o_c = fix.collision.query(oncom);
	auto dist = utils::distance(m_m.pos, o_m.pos); // is squared
	auto radsum = m_c.shape.radius + o_c.shape.radius;
	BOOST_CHECK_LE(radsum * radsum, dist);
}

BOOST_AUTO_TEST_CASE(indirect_tunnelingtest) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();

	auto mover = fix.add_object(fix.scene, {1u, 2u}, { 1, 0}, 1.f, core::MAX_SPEED);
	auto oncom = fix.add_object(fix.scene, {5u, 3u}, {-1, 0}, 1.f, core::MAX_SPEED);
	auto& m_c = fix.collision.query(mover);
	auto& o_c = fix.collision.query(oncom);
	m_c.shape.radius = 0.75f;
	o_c.shape.radius = 0.75f;
	fix.move_object(mover, {1.f, 0.f}, {1.f, 0.f});
	fix.move_object(oncom, {-1.f, 0.f}, {1.f, 0.f});
	fix.update(sf::seconds(8.f));

	// expect collisions
	auto const& colls = fix.collisions;
	BOOST_REQUIRE_EQUAL(colls.size(), 2u);
	BOOST_CHECK_EQUAL(colls[0].actor, oncom);
	BOOST_CHECK_EQUAL(colls[0].collider, mover);
	BOOST_CHECK_EQUAL(colls[1].actor, mover);
	BOOST_CHECK_EQUAL(colls[1].collider, oncom);

	// expect actor has stopped near (3, 2)
	auto const& m_m = fix.movement.query(mover);
	BOOST_CHECK_VECTOR_EQUAL(m_m.move, sf::Vector2i());
	BOOST_CHECK_VECTOR_CLOSE(m_m.pos, sf::Vector2f(2.f, 2.f), 0.001f);
	
	// and expect oncomming object has stopped too
	auto const& o_m = fix.movement.query(oncom);
	BOOST_CHECK_VECTOR_EQUAL(o_m.move, sf::Vector2i());
	BOOST_CHECK_VECTOR_CLOSE(o_m.pos, sf::Vector2f(5.f, 3.f), 0.001f);
	
	// check distance
	auto dist = utils::distance(m_m.pos, o_m.pos); // is squared
	auto radsum = m_c.shape.radius + o_c.shape.radius;
	BOOST_CHECK_LT(radsum * radsum, dist);
}

BOOST_AUTO_TEST_CASE(
	object_stops_movement_if_it_collides_with_an_oncomming_object_in_case_of_even_tiles_in_between) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();

	auto mover = fix.add_object(fix.scene, {2u, 2u}, {-1, 0}, 1.f, 5.f);
	auto oncom = fix.add_object(fix.scene, {5u, 2u}, {-1, 0}, 1.f, 5.f);
	fix.move_object(mover, {1.f, 0.f}, {1.f, 0.f});
	fix.move_object(oncom, {-1.f, 0.f}, {1.f, 0.f});
	fix.update(sf::seconds(6.f));

	// expect collisoins
	auto const& colls = fix.collisions;
	BOOST_REQUIRE_EQUAL(colls.size(), 2u);
	BOOST_CHECK_EQUAL(colls[0].actor, mover);
	BOOST_CHECK_EQUAL(colls[0].collider, oncom);
	BOOST_CHECK_EQUAL(colls[1].actor, oncom);
	BOOST_CHECK_EQUAL(colls[1].collider, mover);

	// expect actor has stopped near (3, 2)
	auto const& m_m = fix.movement.query(mover);
	BOOST_CHECK_VECTOR_EQUAL(m_m.move, sf::Vector2i());
	BOOST_CHECK_VECTOR_CLOSE(m_m.pos, sf::Vector2f(3.f, 2.f), 0.001f);
	
	// and expect oncomming object has stopped too
	auto const& o_m = fix.movement.query(oncom);
	BOOST_CHECK_VECTOR_EQUAL(o_m.move, sf::Vector2i());
	BOOST_CHECK_VECTOR_CLOSE(o_m.pos, sf::Vector2f(4.081f, 2.f), 0.001f);
	
	// check distance
	auto const & m_c = fix.collision.query(mover);
	auto const & o_c = fix.collision.query(oncom);
	auto dist = utils::distance(m_m.pos, o_m.pos); // is squared
	auto radsum = m_c.shape.radius + o_c.shape.radius;
	BOOST_CHECK_LT(radsum * radsum, dist);
}

BOOST_AUTO_TEST_CASE(object_is_not_stopped_if_bullet_collides_with_it) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();

	auto actor = fix.add_object(fix.scene, {3u, 1u}, {1, 0}, 5.f, 5.f);
	auto bullet = fix.add_bullet(fix.scene, {5u, 1u}, {-1, 0}, 5.f, 5.f);
	fix.move_object(actor, {1.f, 0.f}, {1.f, 0.f});
	// object moves to (4,1), bullet moves to (4,1) and collides
	fix.update(sf::seconds(0.5f));

	// expect collisions
	auto& coll = fix.collisions;
	BOOST_REQUIRE_EQUAL(coll.size(), 1u);
	BOOST_CHECK_EQUAL(coll[0].actor, bullet);
	BOOST_CHECK_EQUAL(coll[0].collider, actor);

	// expect both moving on!
	auto& m_a = fix.movement.query(actor);
	BOOST_CHECK(m_a.move != sf::Vector2f{});
	BOOST_CHECK_VECTOR_EQUAL(m_a.move, sf::Vector2i(1, 0));
	BOOST_CHECK_GT(m_a.pos.x, 4.f);
	BOOST_CHECK_CLOSE(m_a.pos.y, 1.f, 0.0001f);
	auto& m_b = fix.movement.query(bullet);
	BOOST_CHECK(m_b.move != sf::Vector2f{});
	BOOST_CHECK_VECTOR_EQUAL(m_b.move, sf::Vector2i(-1, 0));
	BOOST_CHECK_LT(m_b.pos.x, 4.f);
	BOOST_CHECK_CLOSE(m_b.pos.y, 1.f, 0.0001f);
}

BOOST_AUTO_TEST_CASE(bullets_do_collide_with_each_other) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();

	auto actor = fix.add_bullet(fix.scene, {3u, 1u}, {1, 0}, 5.f, 5.f);
	auto other = fix.add_bullet(fix.scene, {5u, 1u}, {-1, 0}, 5.f, 5.f);
	fix.update(sf::seconds(0.5f));

	// expect collisions
	BOOST_CHECK_EQUAL(fix.collisions.size(), 2u);

	// expect both still moving on!
	auto& m_a = fix.movement.query(actor);
	BOOST_CHECK_VECTOR_EQUAL(m_a.move, sf::Vector2i(1, 0));
	BOOST_CHECK_GT(m_a.pos.x, 4.f);
	BOOST_CHECK_CLOSE(m_a.pos.y, 1.f, 0.0001f);
	auto& m_b = fix.movement.query(other);
	BOOST_CHECK_VECTOR_EQUAL(m_b.move, sf::Vector2i(-1, 0));
	BOOST_CHECK_LT(m_b.pos.x, 4.f);
	BOOST_CHECK_CLOSE(m_b.pos.y, 1.f, 0.0001f);
}

// ---------------------------------------------------------------------------
// --- ADVANCED MOVEMENT TESTS

BOOST_AUTO_TEST_CASE(collision_map_is_consistant_after_each_frame) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();

	auto actor = fix.add_object(fix.scene, {1u, 1u}, {1, 0}, 5.f, 5.f);
	auto const& dungeon = fix.dungeon[fix.scene];
	auto const& move = fix.movement.query(actor);

	auto isConsistant = [&]() {
		auto& cell = dungeon.getCell(sf::Vector2u{move.pos});
		return utils::contains(cell.entities, move.id);
	};

	// move over 4 tiles to SE
	std::size_t counter{0u};
	fix.move_object(actor, {1.f, 1.f}, {1.f, 0.f});
	do {
		fix.update(sf::milliseconds(10));
		if (!isConsistant()) {
			BOOST_FAIL("Object should be located at " + thor::toString(sf::Vector2u(move.pos)) + " but it is not.");
		}
		++counter;
	} while (counter < 4u);

	// move over 4 tiles to W
	counter = 0u;
	fix.move_object(actor, {-1.f, 0.f}, {1.f, 0.f});
	do {
		fix.update(sf::milliseconds(10));
		if (!isConsistant()) {
			BOOST_FAIL("Object should be located at " + thor::toString(sf::Vector2u(move.pos)) + " but it is not.");
		}
		++counter;
	} while (counter < 4u);

	// move N until (3,2)
	counter = 0u;
	fix.move_object(actor, {0.f, -1.f}, {1.f, 0.f});
	do {
		fix.update(sf::milliseconds(10));
		if (!isConsistant()) {
			BOOST_FAIL("Object should be located at " + thor::toString(sf::Vector2u(move.pos)) + " but it is not.");
		}
		++counter;
	} while (counter < 4u);
	
	// move backwards
	counter = 0u;
	fix.move_object(actor, {-1.f, -1.f}, {1.f, 1.f});
	do {
		fix.update(sf::milliseconds(10));
		if (!isConsistant()) {
			BOOST_FAIL("Object should be located at " + thor::toString(sf::Vector2u(move.pos)) + " but it is not.");
		}
		++counter;
	} while (counter < 4u);
}

BOOST_AUTO_TEST_CASE(collision_map_works_correct_if_object_collides_with_dungeon_border) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();

	auto actor = fix.add_object(fix.scene, {3u, 2u}, {1, 0}, 5.f, 5.f);
	auto const& dungeon = fix.dungeon[fix.scene];
	auto const& move = fix.movement.query(actor);

	// look right and move backwards (towards end-of-dungeon)
	fix.move_object(actor, {-1.f, 0.f}, {1.f, 0.f});
	fix.update(sf::milliseconds(3000));

	// expect some suitable position
	BOOST_CHECK_GT(move.pos.x, 0.f);
	BOOST_CHECK_LT(move.pos.x, 2.f);
	BOOST_CHECK_GT(move.pos.y, 0.f);
	BOOST_CHECK_LT(move.pos.y, 1.f);
}

BOOST_AUTO_TEST_CASE(teleport_event_is_propagated_on_teleport) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();

	// create teleport trigger
	fix.addTeleport(1u, {4u, 1u}, 1u, {3u, 5u});

	auto mover = fix.add_object(fix.scene, {1u, 1u}, {1, 0}, 1.f, 5.f);
	fix.move_object(mover, {1.f, 0.f}, {-1.f, 1.f});
	
	fix.update(sf::seconds(16.f));

	BOOST_CHECK_EQUAL(fix.teleports.size(), 1u);
}

BOOST_AUTO_TEST_CASE(object_is_not_stopped_after_teleport) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();

	// create teleport trigger
	fix.addTeleport(1u, {4u, 1u}, 1u, {3u, 5u});

	auto mover = fix.add_object(fix.scene, {1u, 1u}, {1, 0}, 1.f, 5.f);
	fix.move_object(mover, {1.f, 0.f}, {-1.f, 1.f});
	fix.update(sf::seconds(1.f));

	// expect object move off teleport target position
	auto const& move_data = fix.movement.query(mover);
	BOOST_CHECK_VECTOR_EQUAL(move_data.move, sf::Vector2f(1.f, 0.f));
	BOOST_CHECK_VECTOR_CLOSE(move_data.pos, sf::Vector2f(4.f, 5.f), 0.0001f);
}

BOOST_AUTO_TEST_CASE(bullet_is_not_effected_by_teleport) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();

	// create teleport trigger
	fix.addTeleport(1u, {4u, 1u}, 1u, {3u, 5u});
	
	auto mover = fix.add_bullet(fix.scene, {1u, 1u}, {1, 0}, 0.f, 5.f);
	fix.update(sf::seconds(8.f));

	// expect object move beyond trigger and collide with wall
	auto const& move_data = fix.movement.query(mover);
	BOOST_CHECK_EQUAL(move_data.scene, fix.scene);
	BOOST_CHECK_VECTOR_EQUAL(move_data.move, sf::Vector2f());
	BOOST_CHECK_VECTOR_CLOSE(move_data.pos, sf::Vector2f(8.f, 1.f), 0.0001f);
}

BOOST_AUTO_TEST_SUITE_END()

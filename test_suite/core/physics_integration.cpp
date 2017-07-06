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
		, focus{log, 1000u, dungeon, movement} {
		// connect events
		collision.bind<core::CollisionEvent>(movement);
		collision.bind<core::CollisionEvent>(*this);
		collision.bind<core::MoveEvent>(focus);
		collision.bind<core::TeleportEvent>(*this);
		movement.bind<core::MoveEvent>(collision);
		movement.bind<core::MoveEvent>(*this);
		focus.bind<core::FocusEvent>(*this);

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
			dynamic_cast<core::MoveSender&>(collision),
			dynamic_cast<core::TeleportSender&>(collision), movement,
			collision, dungeon, to, dst);
	}

	core::ObjectID add_object(utils::SceneID scene, sf::Vector2u const& pos,
		sf::Vector2i const& look, float sight, float max_speed) {
		auto id = id_manager.acquire();
		ids.push_back(id);
		auto& f = focus.acquire(id);
		f.look = look;
		f.sight = sight;
		if (f.sight > 0.f) {
			f.display_name = "foo";
		}
		auto& m = movement.acquire(id);
		m.pos = sf::Vector2f{pos};
		m.target = pos;
		m.scene = scene;
		m.max_speed = max_speed;
		collision.acquire(id);
		auto& d = dungeon[scene];
		d.getCell(pos).entities.push_back(id);
		publish_object(id, pos, scene);
		return id;
	}

	core::ObjectID add_bullet(utils::SceneID scene, sf::Vector2u const& pos,
		sf::Vector2i const& look, float sight, float max_speed) {
		auto id = id_manager.acquire();
		ids.push_back(id);
		auto& f = focus.acquire(id);
		f.look = look;
		f.sight = sight;
		auto& m = movement.acquire(id);
		m.pos = sf::Vector2f{pos};
		m.target = pos;
		m.scene = scene;
		m.max_speed = max_speed;
		auto& c = collision.acquire(id);
		c.is_projectile = true;
		auto& d = dungeon[scene];
		d.getCell(pos).entities.push_back(id);
		publish_object(id, pos, scene);
		move_object(id, look, look);
		return id;
	}

	void publish_object(
		core::ObjectID id, sf::Vector2u const& pos, utils::SceneID scene) {
		core::MoveEvent event;
		event.actor = id;
		event.target = pos;
		event.type = core::MoveEvent::Left;
		focus.receive(event);
	}

	void move_object(
		core::ObjectID id, sf::Vector2i const& move, sf::Vector2i const& look) {
		core::InputEvent event;
		event.actor = id;
		event.move = move;
		event.look = look;
		movement.receive(event);
		focus.receive(event);
	}

	void rotate_object(core::ObjectID id, sf::Vector2i const& look) {
		core::InputEvent event;
		event.actor = id;
		event.look = look;
		focus.receive(event);
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
	fix.update(sf::seconds(6.f));

	// expect object collision between bullet and entity
	auto const& colls = fix.collisions;
	BOOST_REQUIRE_EQUAL(colls.size(), 1u);
	BOOST_CHECK_EQUAL(colls[0].actor, bullet);
	BOOST_CHECK_EQUAL(colls[0].collider, entity);
	BOOST_CHECK(!colls[0].reset);

	// expect bullet moving on!
	auto const& e_m = fix.movement.query(entity);
	auto const& b_m = fix.movement.query(bullet);
	BOOST_CHECK_VECTOR_EQUAL(b_m.next_move, sf::Vector2i(-1, 0));
	BOOST_CHECK_LT(b_m.pos.x, e_m.pos.x);
}

BOOST_AUTO_TEST_CASE(bullet_stops_movement_if_it_hits_an_unaccessable_tile) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();

	auto bullet = fix.add_bullet(fix.scene, {3u, 2u}, {-1, 0}, 1.f, 5.f);
	fix.update(sf::seconds(7.f));

	// expect tile collision
	auto const& colls = fix.collisions;
	BOOST_REQUIRE_EQUAL(colls.size(), 1u);
	BOOST_CHECK_EQUAL(colls[0].actor, bullet);
	BOOST_CHECK_EQUAL(colls[0].collider, 0u);
	BOOST_CHECK(colls[0].reset);
	BOOST_CHECK_VECTOR_EQUAL(colls[0].pos, sf::Vector2u(0u, 2u));
	BOOST_CHECK_VECTOR_EQUAL(colls[0].reset_to, sf::Vector2u(0u, 2u));

	// expect bullet has stopped at (0,2)
	auto const& b_m = fix.movement.query(bullet);
	BOOST_CHECK_VECTOR_EQUAL(b_m.move, sf::Vector2i());
	BOOST_CHECK_VECTOR_CLOSE(b_m.pos, sf::Vector2f(0.f, 2.f), 0.0001f);
}

BOOST_AUTO_TEST_CASE(
	object_stops_movement_if_it_collides_with_standing_object) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();

	auto mover = fix.add_object(fix.scene, {3u, 2u}, {-1, 0}, 1.f, 5.f);
	auto idler = fix.add_object(fix.scene, {3u, 4u}, {-1, 0}, 1.f, 5.f);
	fix.move_object(mover, {0, 1}, {0, 1});
	fix.update(sf::seconds(8.f));

	// expect object collision
	auto const& colls = fix.collisions;
	BOOST_REQUIRE_EQUAL(colls.size(), 1u);
	BOOST_CHECK_EQUAL(colls[0].actor, mover);
	BOOST_CHECK_EQUAL(colls[0].collider, idler);
	BOOST_CHECK(colls[0].reset);
	BOOST_CHECK_VECTOR_EQUAL(colls[0].pos, sf::Vector2u(3u, 4u));
	BOOST_CHECK_VECTOR_EQUAL(colls[0].reset_to, sf::Vector2u(3u, 3u));

	// expect mover has stopped at (3, 3)
	auto const& m_m = fix.movement.query(mover);
	BOOST_CHECK_VECTOR_EQUAL(m_m.move, sf::Vector2i());
	BOOST_CHECK_VECTOR_CLOSE(m_m.pos, sf::Vector2f(3.f, 3.f), 0.0001f);
}

BOOST_AUTO_TEST_CASE(
	object_stops_movement_if_it_collides_directly_neighbored_object) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();

	auto mover = fix.add_object(fix.scene, {3u, 2u}, {-1, 0}, 1.f, 5.f);
	auto idler = fix.add_object(fix.scene, {3u, 3u}, {-1, 0}, 1.f, 5.f);
	fix.move_object(mover, {0, 1}, {0, 1});
	fix.update(sf::seconds(8.f));

	// expect object collision
	auto const& colls = fix.collisions;
	BOOST_REQUIRE_EQUAL(colls.size(), 1u);
	BOOST_CHECK_EQUAL(colls[0].actor, mover);
	BOOST_CHECK_EQUAL(colls[0].collider, idler);
	BOOST_CHECK(colls[0].reset);
	BOOST_CHECK_VECTOR_EQUAL(colls[0].pos, sf::Vector2u(3u, 3u));
	BOOST_CHECK_VECTOR_EQUAL(colls[0].reset_to, sf::Vector2u(3u, 2u));

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
	fix.move_object(object, {0, -1}, {0, 1});
	fix.update(sf::seconds(8.f));

	// expect object collision
	auto const& colls = fix.collisions;
	BOOST_REQUIRE_EQUAL(colls.size(), 1u);
	BOOST_CHECK_EQUAL(colls[0].actor, object);
	BOOST_CHECK_EQUAL(colls[0].collider, 0u);
	BOOST_CHECK(colls[0].reset);
	BOOST_CHECK_VECTOR_EQUAL(colls[0].pos, sf::Vector2u(3u, 0u));
	BOOST_CHECK_VECTOR_EQUAL(colls[0].reset_to, sf::Vector2u(3u, 1u));

	// expect mover has stopped at (3, 1)
	auto const& o_m = fix.movement.query(object);
	BOOST_CHECK_VECTOR_EQUAL(o_m.move, sf::Vector2i());
	BOOST_CHECK_VECTOR_CLOSE(o_m.pos, sf::Vector2f(3.f, 1.f), 0.0001f);
}

BOOST_AUTO_TEST_CASE(
	object_stops_movement_if_it_collides_with_a_crossing_object) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();

	// note: cross was created first, so it is served first
	auto cross = fix.add_object(fix.scene, {2u, 4u}, {-1, 0}, 1.f, 5.f);
	auto mover = fix.add_object(fix.scene, {4u, 2u}, {-1, 0}, 1.f, 5.f);
	// .. despite whose event was created earlier this frame
	fix.move_object(cross, {1, 0}, {1, 0});
	fix.move_object(mover, {0, 1}, {0, 1});
	// .. so finally, the crossing object is served first
	fix.update(sf::seconds(8.f));

	// expect actor to collide with crossing object
	auto const& colls = fix.collisions;
	BOOST_REQUIRE_EQUAL(colls.size(), 1u);
	BOOST_CHECK_EQUAL(colls[0].actor, mover);
	BOOST_CHECK_EQUAL(colls[0].collider, cross);
	BOOST_CHECK(colls[0].reset);
	BOOST_CHECK_VECTOR_EQUAL(colls[0].pos, sf::Vector2u(4u, 4u));
	BOOST_CHECK_VECTOR_EQUAL(colls[0].reset_to, sf::Vector2u(4u, 3u));

	// expect actor object has stopped at (4, 3)
	auto const& m_m = fix.movement.query(mover);
	BOOST_CHECK_VECTOR_EQUAL(m_m.move, sf::Vector2i());
	BOOST_CHECK_VECTOR_CLOSE(m_m.pos, sf::Vector2f(4.f, 3.f), 0.0001f);

	// expect crossing object has passed (4, 3) to the east
	auto const& c_m = fix.movement.query(cross);
	BOOST_CHECK_VECTOR_EQUAL(c_m.next_move, sf::Vector2i(1, 0));
	BOOST_CHECK_CLOSE(c_m.pos.y, 4.f, 0.0001f);
	BOOST_CHECK_GT(c_m.pos.x, 4.f);
}

BOOST_AUTO_TEST_CASE(
	object_stops_movement_if_it_collides_with_an_oncomming_object_in_case_of_odd_tiles_in_between) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();

	auto mover = fix.add_object(fix.scene, {1u, 2u}, {-1, 0}, 1.f, 5.f);
	auto oncom = fix.add_object(fix.scene, {5u, 2u}, {-1, 0}, 1.f, 5.f);
	fix.move_object(mover, {1, 0}, {1, 0});
	fix.move_object(oncom, {-1, 0}, {1, 0});
	fix.update(sf::seconds(8.f));

	// expect oncomming object collide with actor
	auto const& colls = fix.collisions;
	BOOST_REQUIRE_EQUAL(colls.size(), 2u);
	BOOST_CHECK_EQUAL(colls[0].actor, oncom);
	BOOST_CHECK_EQUAL(colls[0].collider, mover);
	BOOST_CHECK(colls[0].reset);
	BOOST_CHECK_VECTOR_EQUAL(colls[0].pos, sf::Vector2u(3u, 2u));
	BOOST_CHECK_VECTOR_EQUAL(colls[0].reset_to, sf::Vector2u(4u, 2u));
	// and expect actor to collide with oncomming object
	BOOST_CHECK_EQUAL(colls[1].actor, mover);
	BOOST_CHECK_EQUAL(colls[1].collider, oncom);
	BOOST_CHECK(colls[1].reset);
	BOOST_CHECK_VECTOR_EQUAL(colls[1].pos, sf::Vector2u(4u, 2u));
	BOOST_CHECK_VECTOR_EQUAL(colls[1].reset_to, sf::Vector2u(3u, 2u));

	// expect actor has stopped at (3, 2)
	auto const& m_m = fix.movement.query(mover);
	BOOST_CHECK_VECTOR_EQUAL(m_m.move, sf::Vector2i());
	BOOST_CHECK_VECTOR_CLOSE(m_m.pos, sf::Vector2f(3.f, 2.f), 0.0001f);
	// and expect oncomming object has stopped at (4, 2)
	auto const& o_m = fix.movement.query(oncom);
	BOOST_CHECK_VECTOR_EQUAL(o_m.move, sf::Vector2i());
	BOOST_CHECK_VECTOR_CLOSE(o_m.pos, sf::Vector2f(4.f, 2.f), 0.0001f);
}

BOOST_AUTO_TEST_CASE(
	object_stops_movement_if_it_collides_with_an_oncomming_object_in_case_of_even_tiles_in_between) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();

	auto mover = fix.add_object(fix.scene, {2u, 2u}, {-1, 0}, 1.f, 5.f);
	auto oncom = fix.add_object(fix.scene, {5u, 2u}, {-1, 0}, 1.f, 5.f);
	fix.move_object(mover, {1, 0}, {1, 0});
	fix.move_object(oncom, {-1, 0}, {1, 0});
	fix.update(sf::seconds(6.f));

	// expect actor collide with oncomming object
	auto const& colls = fix.collisions;
	BOOST_REQUIRE_EQUAL(colls.size(), 2u);
	BOOST_CHECK_EQUAL(colls[0].actor, mover);
	BOOST_CHECK_EQUAL(colls[0].collider, oncom);
	BOOST_CHECK(colls[0].reset);
	BOOST_CHECK_VECTOR_EQUAL(colls[0].pos, sf::Vector2u(4u, 2u));
	BOOST_CHECK_VECTOR_EQUAL(colls[0].reset_to, sf::Vector2u(3u, 2u));
	// and expect oncomming object collide with actor
	BOOST_CHECK_EQUAL(colls[1].actor, oncom);
	BOOST_CHECK_EQUAL(colls[1].collider, mover);
	BOOST_CHECK(colls[1].reset);
	BOOST_CHECK_VECTOR_EQUAL(colls[1].pos, sf::Vector2u(3u, 2u));
	BOOST_CHECK_VECTOR_EQUAL(colls[1].reset_to, sf::Vector2u(4u, 2u));

	// expect actor has stopped at (3, 2)
	auto const& m_m = fix.movement.query(mover);
	BOOST_CHECK_VECTOR_EQUAL(m_m.move, sf::Vector2i());
	BOOST_CHECK_VECTOR_CLOSE(m_m.pos, sf::Vector2f(3.f, 2.f), 0.0001f);
	// and expect oncomming object has stopped at (4, 2)
	auto const& o_m = fix.movement.query(oncom);
	BOOST_CHECK_VECTOR_EQUAL(o_m.move, sf::Vector2i());
	BOOST_CHECK_VECTOR_CLOSE(o_m.pos, sf::Vector2f(4.f, 2.f), 0.0001f);
}

BOOST_AUTO_TEST_CASE(object_is_stopped_if_bullet_collides_with_it) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();

	auto actor = fix.add_object(fix.scene, {3u, 1u}, {1, 0}, 5.f, 5.f);
	auto bullet = fix.add_bullet(fix.scene, {5u, 1u}, {-1, 0}, 5.f, 5.f);
	fix.move_object(actor, {1, 0}, {1, 0});
	// object moves to (4,1), bullet moves to (4,1) and collides
	fix.update(sf::seconds(3.f));

	// expect bullet collide with actor
	auto& coll = fix.collisions;
	BOOST_REQUIRE_EQUAL(coll.size(), 1u);
	BOOST_CHECK_EQUAL(coll[0].actor, bullet);
	BOOST_CHECK_EQUAL(coll[0].collider, actor);
	BOOST_CHECK(!coll[0].reset);

	// expect both moving on!
	auto& m_a = fix.movement.query(actor);
	BOOST_CHECK_VECTOR_EQUAL(m_a.move, sf::Vector2i(1, 0));
	BOOST_CHECK_GT(m_a.pos.x, 4.f);
	BOOST_CHECK_CLOSE(m_a.pos.y, 1.f, 0.0001f);
	auto& m_b = fix.movement.query(bullet);
	BOOST_CHECK_VECTOR_EQUAL(m_b.move, sf::Vector2i(-1, 0));
	BOOST_CHECK_LT(m_b.pos.x, 4.f);
	BOOST_CHECK_CLOSE(m_b.pos.y, 1.f, 0.0001f);
}

BOOST_AUTO_TEST_CASE(bullets_do_not_collide_with_each_other) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();

	auto actor = fix.add_bullet(fix.scene, {3u, 1u}, {1, 0}, 5.f, 5.f);
	auto other = fix.add_bullet(fix.scene, {5u, 1u}, {-1, 0}, 5.f, 5.f);
	fix.update(sf::seconds(3.f));

	// expect no collisions
	BOOST_CHECK(fix.collisions.empty());

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
// --- FOCUS TESTS

BOOST_AUTO_TEST_CASE(object_gains_focus_when_facing_another_object) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();

	auto actor = fix.add_object(fix.scene, {2u, 2u}, {-1, 0}, 10.f, 5.f);
	auto obser = fix.add_object(fix.scene, {5u, 2u}, {0, 1}, 10.f, 5.f);
	fix.rotate_object(actor, {1, 0});
	fix.update(sf::seconds(1.f));

	// expect actor gained focus to observer
	BOOST_REQUIRE_EQUAL(fix.focuses.size(), 1u);
	BOOST_CHECK_EQUAL(fix.focuses[0].observer, actor);
	BOOST_CHECK_EQUAL(fix.focuses[0].observed, obser);
	BOOST_CHECK(fix.focuses[0].type == core::FocusEvent::Gained);

	// expect being focused
	auto& a_f = fix.focus.query(actor);
	auto& o_f = fix.focus.query(obser);
	BOOST_CHECK_EQUAL(a_f.focus, obser);
	BOOST_REQUIRE_EQUAL(o_f.observers.size(), 1u);
	BOOST_CHECK_EQUAL(o_f.observers[0], actor);
}

BOOST_AUTO_TEST_CASE(object_gains_no_focus_when_facing_bullet) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();

	auto actor = fix.add_object(fix.scene, {2u, 2u}, {-1, 0}, 10.f, 5.f);
	fix.add_bullet(fix.scene, {5u, 2u}, {0, 1}, 0.f, 5.f);
	fix.rotate_object(actor, {1, 0});
	fix.update(sf::seconds(1.f));

	// expect actor did not focus bullet
	BOOST_CHECK(fix.focuses.empty());
}

BOOST_AUTO_TEST_CASE(object_loses_focus_when_facing_into_void) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();

	auto actor = fix.add_object(fix.scene, {2u, 2u}, {-1, 0}, 10.f, 5.f);
	auto obser = fix.add_object(fix.scene, {5u, 2u}, {0, 1}, 10.f, 5.f);
	fix.rotate_object(actor, {1, 0});
	fix.update(sf::seconds(1.f));
	fix.rotate_object(actor, {0, 1});
	fix.update(sf::seconds(1.f));

	// expect actor gained focus to observer
	BOOST_REQUIRE_EQUAL(2u, fix.focuses.size());
	BOOST_CHECK_EQUAL(fix.focuses[1].observer, actor);
	BOOST_CHECK_EQUAL(fix.focuses[1].observed, obser);
	BOOST_CHECK(fix.focuses[1].type == core::FocusEvent::Lost);

	// expect being unfocused
	auto& a_f = fix.focus.query(actor);
	auto& o_f = fix.focus.query(obser);
	BOOST_CHECK_EQUAL(a_f.focus, 0u);
	BOOST_CHECK(o_f.observers.empty());
}

BOOST_AUTO_TEST_CASE(object_gains_focus_when_another_object_moves_into_fov) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();

	auto actor = fix.add_object(fix.scene, {2u, 2u}, {0, 1}, 10.f, 5.f);
	auto mover = fix.add_object(fix.scene, {4u, 3u}, {-1, 0}, 10.f, 5.f);
	fix.move_object(mover, {-1, 0}, {-1, 0});
	fix.update(sf::seconds(3.f));

	// expect actor gained focus to observer
	BOOST_REQUIRE_EQUAL(1u, fix.focuses.size());
	BOOST_CHECK_EQUAL(fix.focuses[0].observer, actor);
	BOOST_CHECK_EQUAL(fix.focuses[0].observed, mover);
	BOOST_CHECK(fix.focuses[0].type == core::FocusEvent::Gained);

	// expect being focused
	auto& a_f = fix.focus.query(actor);
	auto& m_f = fix.focus.query(mover);
	BOOST_CHECK_EQUAL(a_f.focus, mover);
	BOOST_REQUIRE_EQUAL(m_f.observers.size(), 1u);
	BOOST_CHECK_EQUAL(m_f.observers[0], actor);
}

BOOST_AUTO_TEST_CASE(object_loses_focus_when_another_object_moves_out_of_fov) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();

	auto actor = fix.add_object(fix.scene, {2u, 2u}, {0, 1}, 10.f, 5.f);
	auto mover = fix.add_object(fix.scene, {2u, 4u}, {0, 1}, 10.f, 5.f);
	fix.update(sf::milliseconds(10));  // to update focus
	fix.move_object(mover, {1, 0}, {1, 0});
	fix.update(sf::seconds(3.f));

	// expect actor gained focus to observer
	BOOST_REQUIRE_EQUAL(2u, fix.focuses.size());
	BOOST_CHECK_EQUAL(fix.focuses[1].observer, actor);
	BOOST_CHECK_EQUAL(fix.focuses[1].observed, mover);
	BOOST_CHECK(fix.focuses[1].type == core::FocusEvent::Lost);

	// expect being focused
	auto& a_f = fix.focus.query(actor);
	auto& m_f = fix.focus.query(mover);
	BOOST_CHECK_EQUAL(a_f.focus, 0u);
	BOOST_CHECK(m_f.observers.empty());
}

BOOST_AUTO_TEST_CASE(
	object_temporary_gains_and_loses_focus_as_another_object_passes_by) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();

	auto actor = fix.add_object(fix.scene, {3u, 2u}, {0, 1}, 10.f, 5.f);
	auto mover = fix.add_object(fix.scene, {1u, 4u}, {0, 1}, 10.f, 5.f);
	fix.move_object(mover, {1, 0}, {1, 0});
	fix.update(sf::seconds(8.f));

	// expect actor gained focus to observer
	BOOST_REQUIRE_EQUAL(2u, fix.focuses.size());
	BOOST_CHECK_EQUAL(fix.focuses[0].observer, actor);
	BOOST_CHECK_EQUAL(fix.focuses[0].observed, mover);
	BOOST_CHECK(fix.focuses[0].type == core::FocusEvent::Gained);
	BOOST_CHECK_EQUAL(fix.focuses[1].observer, actor);
	BOOST_CHECK_EQUAL(fix.focuses[1].observed, mover);
	BOOST_CHECK(fix.focuses[1].type == core::FocusEvent::Lost);

	// expect being focused
	auto& a_f = fix.focus.query(actor);
	auto& m_f = fix.focus.query(mover);
	BOOST_CHECK_EQUAL(a_f.focus, 0u);
	BOOST_CHECK(m_f.observers.empty());
}


BOOST_AUTO_TEST_CASE(object_focuses_closest_object_if_multiple_are_moving_to_view) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();

	auto actor = fix.add_object(fix.scene, {1u, 1u}, {1, 0}, 10.f, 5.f);
	auto second = fix.add_object(fix.scene, {2u, 1u}, {0, 1}, 10.f, 5.f);
	auto third = fix.add_object(fix.scene, {3u, 2u}, {0, 1}, 10.f, 5.f);
	fix.update(sf::milliseconds(10));
	
	// expect focus towards second
	auto const & focus = fix.focus.query(actor);
	BOOST_CHECK_EQUAL(focus.focus, second);
	
	// let third move to same row
	fix.move_object(third, {0, -1}, {0, -1});
	fix.update(sf::milliseconds(100));
	
	// expect focus towards second
	BOOST_CHECK_EQUAL(focus.focus, second);
}

BOOST_AUTO_TEST_CASE(
	focus_is_restored_after_another_object_passed_through_fov) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();

	auto actor = fix.add_object(fix.scene, {3u, 2u}, {0, 1}, 10.f, 5.f);
	auto mover = fix.add_object(fix.scene, {1u, 3u}, {1, 0}, 10.f, 5.f);
	auto third = fix.add_object(fix.scene, {3u, 5u}, {0, 1}, 10.f, 5.f);
	fix.move_object(mover, {1, 0}, {1, 0});
	fix.update(sf::seconds(8.f));

	// expect actor gained to third object
	BOOST_REQUIRE_EQUAL(5u, fix.focuses.size());
	BOOST_CHECK_EQUAL(fix.focuses[0].observer, actor);
	BOOST_CHECK_EQUAL(fix.focuses[0].observed, third);
	BOOST_CHECK(fix.focuses[0].type == core::FocusEvent::Gained);
	// expect actor to lose focus to third and focus mover
	BOOST_CHECK_EQUAL(fix.focuses[1].observer, actor);
	BOOST_CHECK_EQUAL(fix.focuses[1].observed, third);
	BOOST_CHECK(fix.focuses[1].type == core::FocusEvent::Lost);
	BOOST_CHECK_EQUAL(fix.focuses[2].observer, actor);
	BOOST_CHECK_EQUAL(fix.focuses[2].observed, mover);
	BOOST_CHECK(fix.focuses[2].type == core::FocusEvent::Gained);
	// expect actor to lose focus to mover and refocus third
	BOOST_CHECK_EQUAL(fix.focuses[3].observer, actor);
	BOOST_CHECK_EQUAL(fix.focuses[3].observed, mover);
	BOOST_CHECK(fix.focuses[3].type == core::FocusEvent::Lost);
	BOOST_CHECK_EQUAL(fix.focuses[4].observer, actor);
	BOOST_CHECK_EQUAL(fix.focuses[4].observed, third);
	BOOST_CHECK(fix.focuses[4].type == core::FocusEvent::Gained);

	// expect being focused
	auto& a_f = fix.focus.query(actor);
	auto& m_f = fix.focus.query(mover);
	auto& t_f = fix.focus.query(third);
	BOOST_CHECK_EQUAL(a_f.focus, third);
	BOOST_CHECK(m_f.observers.empty());
	BOOST_REQUIRE_EQUAL(t_f.observers.size(), 1u);
	BOOST_CHECK_EQUAL(t_f.observers[0], actor);
}

BOOST_AUTO_TEST_CASE(objects_gain_focus_while_moving_towards_each_other) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();

	auto actor = fix.add_object(fix.scene, {1u, 1u}, {1, 0}, 5.f, 5.f);
	auto other = fix.add_object(fix.scene, {6u, 1u}, {-1, 0}, 5.f, 5.f);
	fix.move_object(actor, {1, 0}, {1, 0});
	fix.move_object(other, {-1, 0}, {-1, 0});
	fix.update(sf::milliseconds(20));

	// expect both gained focus to each other
	// note: actor is served first because he was created first
	auto& focus = fix.focuses;
	BOOST_REQUIRE_EQUAL(focus.size(), 2u);
	BOOST_CHECK_EQUAL(focus[0].observer, actor);
	BOOST_CHECK_EQUAL(focus[0].observed, other);
	BOOST_CHECK_EQUAL(focus[0].type, core::FocusEvent::Gained);
	BOOST_CHECK_EQUAL(focus[1].observer, other);
	BOOST_CHECK_EQUAL(focus[1].observed, actor);
	BOOST_CHECK_EQUAL(focus[1].type, core::FocusEvent::Gained);

	// expect both focusing each other
	auto& f_a = fix.focus.query(actor);
	auto& f_o = fix.focus.query(other);
	BOOST_CHECK_EQUAL(f_a.focus, other);
	BOOST_CHECK_EQUAL(f_o.focus, actor);
	BOOST_REQUIRE_EQUAL(f_a.observers.size(), 1u);
	BOOST_CHECK_EQUAL(f_a.observers[0], other);
	BOOST_REQUIRE_EQUAL(f_o.observers.size(), 1u);
	BOOST_CHECK_EQUAL(f_o.observers[0], actor);
}

BOOST_AUTO_TEST_CASE(
	objects_lose_focus_while_moving_away_from_each_other_object) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();

	auto actor = fix.add_object(fix.scene, {3u, 1u}, {1, 0}, 5.f, 5.f);
	auto other = fix.add_object(fix.scene, {8u, 1u}, {-1, 0}, 5.f, 5.f);
	fix.update(sf::milliseconds(20));
	auto& focus = fix.focuses;
	// expect both gained focus to each other
	// note: actor is served first because he was created first
	BOOST_REQUIRE_EQUAL(focus.size(), 2u);
	BOOST_CHECK_EQUAL(focus[0].observer, actor);
	BOOST_CHECK_EQUAL(focus[0].observed, other);
	BOOST_CHECK_EQUAL(focus[0].type, core::FocusEvent::Gained);
	BOOST_CHECK_EQUAL(focus[1].observer, other);
	BOOST_CHECK_EQUAL(focus[1].observed, actor);
	BOOST_CHECK_EQUAL(focus[1].type, core::FocusEvent::Gained);

	fix.move_object(actor, {-1, 0}, {1, 0});
	fix.move_object(other, {1, 0}, {-1, 0});
	// update more than one frame to let move events pass through from
	// movement to collision to focus system
	fix.update(sf::seconds(1.f));

	// expect both lost focus to each other
	BOOST_REQUIRE_EQUAL(focus.size(), 4u);
	BOOST_CHECK_EQUAL(focus[2].observer, actor);
	BOOST_CHECK_EQUAL(focus[2].observed, other);
	BOOST_CHECK_EQUAL(focus[2].type, core::FocusEvent::Lost);
	BOOST_CHECK_EQUAL(focus[3].observer, other);
	BOOST_CHECK_EQUAL(focus[3].observed, actor);
	BOOST_CHECK_EQUAL(focus[3].type, core::FocusEvent::Lost);

	// expect both focusing each other
	auto& f_a = fix.focus.query(actor);
	auto& f_o = fix.focus.query(other);
	BOOST_CHECK_EQUAL(f_a.focus, 0);
	BOOST_CHECK_EQUAL(f_o.focus, 0);
	BOOST_CHECK(f_a.observers.empty());
	BOOST_CHECK(f_o.observers.empty());
}

BOOST_AUTO_TEST_CASE(object_gains_focus_by_strifing) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();

	auto actor = fix.add_object(fix.scene, {1u, 2u}, {1, 0}, 5.f, 5.f);
	auto other = fix.add_object(fix.scene, {3u, 1u}, {0, 1}, 5.f, 5.f);
	fix.move_object(actor, {0, -1}, {1, 0});
	// update more than one frame to let move events pass through from
	// movement to collision to focus system
	fix.update(sf::seconds(1.f));

	// expect actor gaining focus via strifing
	auto& focus = fix.focuses;
	BOOST_REQUIRE_EQUAL(focus.size(), 1);
	BOOST_CHECK_EQUAL(focus[0].observer, actor);
	BOOST_CHECK_EQUAL(focus[0].observed, other);
	BOOST_CHECK_EQUAL(focus[0].type, core::FocusEvent::Gained);

	// expect being focused
	auto& f_a = fix.focus.query(actor);
	auto& f_o = fix.focus.query(other);
	BOOST_CHECK_EQUAL(f_a.focus, other);
	BOOST_REQUIRE_EQUAL(f_o.observers.size(), 1u);
	BOOST_CHECK_EQUAL(f_o.observers[0], actor);
}

BOOST_AUTO_TEST_CASE(
	object_does_not_gain_focus_by_strifing_if_collision_blocks_movement) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();

	auto actor = fix.add_object(fix.scene, {1u, 2u}, {1, 0}, 5.f, 5.f);
	auto block = fix.add_object(fix.scene, {1u, 1u}, {-1, 0}, 5.f, 5.f);
	auto other = fix.add_object(fix.scene, {3u, 1u}, {0, 1}, 5.f, 5.f);
	fix.update(sf::milliseconds(10));  // to update focus
	fix.focuses.clear();
	fix.move_object(actor, {0, -1}, {1, 0});
	// update more than one frame to let move events pass through from
	// movement to collision to focus (or even movement) system
	fix.update(sf::seconds(1.f));

	// expect actor collide with blocker
	auto& colls = fix.collisions;
	BOOST_REQUIRE_EQUAL(colls.size(), 1);
	BOOST_CHECK_EQUAL(colls[0].actor, actor);
	BOOST_CHECK_EQUAL(colls[0].collider, block);

	// expect not being focused
	auto& f_a = fix.focus.query(actor);
	auto& f_o = fix.focus.query(other);
	BOOST_CHECK_EQUAL(f_a.focus, 0u);
	BOOST_CHECK(f_o.observers.empty());
}

// ---------------------------------------------------------------------------
// --- ADVANCED MOVEMENT TESTS

BOOST_AUTO_TEST_CASE(collision_map_is_consistant_after_each_frame) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();

	auto actor = fix.add_object(fix.scene, {1u, 1u}, {1, 0}, 5.f, 5.f);
	auto const& dungeon = fix.dungeon[fix.scene];
	auto const& move = fix.movement.query(actor);

	auto hasReached = [&](core::ObjectID id, sf::Vector2u const& pos) {
		for (auto const& m : fix.moves) {
			if (m.type == core::MoveEvent::Reached && m.actor == id &&
				m.target == pos) {
				return true;
			}
		}
		return false;
	};

	auto isConsistant = [&]() {
		auto& cell = dungeon.getCell(move.target);
		return utils::contains(cell.entities, move.id);
	};

	// move SE until (8,8)
	fix.move_object(actor, {1, 1}, {1, 0});
	while (true) {
		fix.update(sf::milliseconds(10));
		if (!isConsistant()) {
			BOOST_FAIL("Object should be located at <" +
					   std::to_string(move.target.x) + "," +
					   std::to_string(move.target.y) + "> but it is not.");
		}
		if (hasReached(actor, {8u, 8u})) {
			break;
		}
	}

	// move W until (3,8)
	fix.move_object(actor, {-1, 0}, {1, 0});
	while (true) {
		fix.update(sf::milliseconds(10));
		if (!isConsistant()) {
			BOOST_FAIL("Object should be located at <" +
					   std::to_string(move.target.x) + "," +
					   std::to_string(move.target.y) + "> but it is not.");
		}
		if (hasReached(actor, {3u, 8u})) {
			break;
		}
	}

	// move N until (3,2)
	fix.move_object(actor, {0, -1}, {1, 0});
	while (true) {
		fix.update(sf::milliseconds(10));
		if (!isConsistant()) {
			BOOST_FAIL("Object should be located at <" +
					   std::to_string(move.target.x) + "," +
					   std::to_string(move.target.y) + "> but it is not.");
		}
		if (hasReached(actor, {3u, 2u})) {
			break;
		}
	}
}

BOOST_AUTO_TEST_CASE(
	object_can_walk_path_by_sending_new_direction_after_tile_was_reached) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();

	auto actor = fix.add_object(fix.scene, {1u, 1u}, {1, 0}, 5.f, 5.f);
	std::vector<sf::Vector2i> directions;
	directions.emplace_back(1, 1);
	directions.emplace_back(0, 1);
	directions.emplace_back(1, 0);
	directions.emplace_back(0, -1);
	directions.emplace_back(-1, -1);

	// walk path
	std::vector<sf::Vector2u> path;
	for (auto i = 0u; i < directions.size(); ++i) {
		// trigger next movement
		auto next = directions[i];
		fix.move_object(actor, next, next);
		// interpolate until tile was reached
		bool reached = false;
		while (!reached) {
			fix.update(sf::milliseconds(10));
			for (auto const& m : fix.moves) {
				switch (m.type) {
					case core::MoveEvent::Left:
						fix.move_object(actor, {}, {});
						break;

					case core::MoveEvent::Reached:
						reached = true;
						path.push_back(m.target);
						break;
				}
				if (reached) {
					break;
				}
			}
			fix.moves.clear();
		}
	}

	// expect exact path
	BOOST_REQUIRE_EQUAL(path.size(), 5u);
	BOOST_CHECK_VECTOR_EQUAL(path[0], sf::Vector2u(2u, 2u));
	BOOST_CHECK_VECTOR_EQUAL(path[1], sf::Vector2u(2u, 3u));
	BOOST_CHECK_VECTOR_EQUAL(path[2], sf::Vector2u(3u, 3u));
	BOOST_CHECK_VECTOR_EQUAL(path[3], sf::Vector2u(3u, 2u));
	BOOST_CHECK_VECTOR_EQUAL(path[4], sf::Vector2u(2u, 1u));
}

BOOST_AUTO_TEST_CASE(teleport_event_is_propagated_on_teleport) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();

	// create teleport trigger
	fix.addTeleport(1u, {4u, 1u}, 1u, {3u, 5u});

	auto mover = fix.add_object(fix.scene, {1u, 1u}, {1, 0}, 1.f, 5.f);
	fix.move_object(mover, {1, 0}, {-1, 1});
	fix.update(sf::seconds(16.f));

	BOOST_CHECK_EQUAL(fix.teleports.size(), 1u);
}

BOOST_AUTO_TEST_CASE(object_is_stopped_after_teleport) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();

	// create teleport trigger
	fix.addTeleport(1u, {4u, 1u}, 1u, {3u, 5u});

	auto mover = fix.add_object(fix.scene, {1u, 1u}, {1, 0}, 1.f, 5.f);
	fix.move_object(mover, {1, 0}, {-1, 1});
	fix.update(sf::seconds(16.f));

	// expect object idle at target position
	auto const& move_data = fix.movement.query(mover);
	BOOST_CHECK_VECTOR_EQUAL(move_data.move, sf::Vector2i());
	BOOST_CHECK_VECTOR_EQUAL(move_data.target, sf::Vector2u(3u, 5u));
	BOOST_CHECK_VECTOR_CLOSE(move_data.target, sf::Vector2f(3.f, 5.f), 0.0001f);
}

BOOST_AUTO_TEST_CASE(bullet_is_not_effected_by_teleport) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();

	// create teleport trigger
	fix.addTeleport(1u, {4u, 1u}, 1u, {3u, 5u});
	
	auto mover = fix.add_bullet(fix.scene, {1u, 1u}, {1, 0}, 0.f, 5.f);
	fix.update(sf::seconds(8.f));

	// expect object move beyond trigger
	auto const& move_data = fix.movement.query(mover);
	BOOST_CHECK_EQUAL(move_data.scene, fix.scene);
	BOOST_CHECK_VECTOR_EQUAL(move_data.move, sf::Vector2i());
	BOOST_CHECK_VECTOR_EQUAL(move_data.target, sf::Vector2u(5u, 1u));
	BOOST_CHECK_VECTOR_CLOSE(move_data.target, sf::Vector2f(5.f, 1.f), 0.0001f);
}

BOOST_AUTO_TEST_CASE(previous_active_focus_is_reset_on_teleport) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();

	// create teleport trigger
	fix.addTeleport(1u, {4u, 1u}, 1u, {3u, 5u});

	auto mover = fix.add_object(fix.scene, {1u, 1u}, {0, 1}, 1.f, 5.f);
	auto other = fix.add_object(fix.scene, {1u, 2u}, {0, 1}, 1.f, 5.f);
	fix.update(sf::milliseconds(150));

	auto const& actor_focus = fix.focus.query(mover);
	auto const& other_focus = fix.focus.query(other);
	BOOST_CHECK_EQUAL(actor_focus.focus, other);
	BOOST_CHECK(utils::contains(other_focus.observers, mover));

	fix.move_object(mover, {1, 0}, {-1, 1});
	fix.update(sf::seconds(8.f));

	// expect focus reset
	BOOST_CHECK_EQUAL(actor_focus.focus, 0u);
	BOOST_CHECK(!utils::contains(other_focus.observers, mover));
}

BOOST_AUTO_TEST_CASE(previous_passive_focus_is_reset_on_teleport) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();

	// create teleport trigger
	fix.addTeleport(1u, {4u, 1u}, 1u, {3u, 5u});

	auto mover = fix.add_object(fix.scene, {1u, 1u}, {1, 0}, 1.f, 5.f);
	auto other = fix.add_object(fix.scene, {1u, 2u}, {0, -1}, 1.f, 5.f);
	fix.update(sf::milliseconds(150));

	auto const& actor_focus = fix.focus.query(mover);
	auto const& other_focus = fix.focus.query(other);
	BOOST_CHECK_EQUAL(other_focus.focus, mover);
	BOOST_CHECK(utils::contains(actor_focus.observers, other));

	fix.move_object(mover, {1, 0}, {-1, 1});
	fix.update(sf::seconds(8.f));

	// expect focus reset
	BOOST_CHECK_EQUAL(other_focus.focus, 0u);
	BOOST_CHECK(!utils::contains(actor_focus.observers, other));
}

BOOST_AUTO_TEST_CASE(active_focus_is_set_on_teleport) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();

	// create teleport trigger
	fix.addTeleport(1u, {4u, 1u}, 1u, {3u, 5u});

	auto mover = fix.add_object(fix.scene, {1u, 1u}, {0, 1}, 1.f, 5.f);
	auto other = fix.add_object(fix.scene, {3u, 6u}, {0, 1}, 1.f, 5.f);

	fix.move_object(mover, {1, 0}, {0, 1});
	fix.update(sf::seconds(8.f));

	// expect focus set
	auto const& actor_focus = fix.focus.query(mover);
	auto const& other_focus = fix.focus.query(other);
	BOOST_CHECK_EQUAL(actor_focus.focus, other);
	BOOST_CHECK(utils::contains(other_focus.observers, mover));
}

BOOST_AUTO_TEST_CASE(passive_focus_is_set_on_teleport) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();

	// create teleport trigger
	fix.addTeleport(1u, {4u, 1u}, 1u, {3u, 5u});

	auto mover = fix.add_object(fix.scene, {1u, 1u}, {0, 1}, 1.f, 5.f);
	auto other = fix.add_object(fix.scene, {3u, 4u}, {0, 1}, 1.f, 5.f);

	fix.move_object(mover, {1, 0}, {0, 1});
	fix.update(sf::seconds(8.f));

	// expect focus set
	auto const& actor_focus = fix.focus.query(mover);
	auto const& other_focus = fix.focus.query(other);
	BOOST_CHECK_EQUAL(other_focus.focus, mover);
	BOOST_CHECK(utils::contains(actor_focus.observers, other));
}

// ---------------------------------------------------------------------------
// --- EVENT SYSTEM TESTS

BOOST_AUTO_TEST_CASE(invalid_collision_event_is_ignored_by_movement_system) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();

	BOOST_REQUIRE(!utils::contains(fix.ids, 1000u));
	core::CollisionEvent event;
	event.actor = 1000u;
	fix.movement.receive(event);
	fix.movement.update(sf::milliseconds(250));
}

BOOST_AUTO_TEST_CASE(invalid_move_event_is_ignored_by_focus_system) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();

	BOOST_REQUIRE(!utils::contains(fix.ids, 1000u));
	core::MoveEvent event;
	event.actor = 1000u;
	fix.focus.receive(event);
	fix.focus.update(sf::milliseconds(250));
}

BOOST_AUTO_TEST_CASE(invalid_move_event_is_ignored_by_collision_system) {
	auto& fix = Singleton<PhysicsFixture>::get();
	fix.reset();

	BOOST_REQUIRE(!utils::contains(fix.ids, 1000u));
	core::MoveEvent event;
	event.actor = 1000u;
	fix.collision.receive(event);
	fix.collision.update(sf::milliseconds(250));
}

BOOST_AUTO_TEST_SUITE_END()

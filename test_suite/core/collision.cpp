#include <vector>
#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>
#include <testsuite/singleton.hpp>

#include <utils/assert.hpp>
#include <utils/algorithm.hpp>
#include <core/collision.hpp>

struct CollisionFixture {
	sf::Texture dummy_tileset;
	core::IdManager id_manager;
	std::vector<core::ObjectID> ids;

	core::LogContext log;
	core::CollisionSender collision_sender;
	core::MoveSender move_sender;
	core::TeleportSender teleport_sender;
	core::CollisionManager collision_manager;
	core::DungeonSystem dungeon_system;
	core::MovementManager movement_manager;
	core::collision_impl::Context context;

	CollisionFixture()
		: dummy_tileset{}
		, id_manager{}
		, log{}
		, collision_sender{}
		, move_sender{}
		, collision_manager{}
		, dungeon_system{}
		, movement_manager{}
		, context{log, collision_sender, move_sender, teleport_sender,
			collision_manager, dungeon_system, movement_manager} {
		// add a scenes
		auto scene = dungeon_system.create(
			dummy_tileset, sf::Vector2u{5u, 6u}, sf::Vector2f{1.f, 1.f});
		assert(scene == 1u);
		auto& dungeon = dungeon_system[1u];
		for (auto y = 1u; y <= 5u; ++y) {
			for (auto x = 1u; x <= 4u; ++x) {
				dungeon.getCell({x, y}).terrain = core::Terrain::Floor;
			}
		}
	}

	void reset() {
		auto& dungeon = dungeon_system[1u];
		// clear dungeon
		for (auto y = 0u; y < 6u; ++y) {
			for (auto x = 0u; x < 5u; ++x) {
				auto& cell = dungeon.getCell({x, y});
				cell.entities.clear();
				cell.trigger = nullptr;
			}
		}
		// remove components
		for (auto id : ids) {
			collision_manager.release(id);
			movement_manager.release(id);
		}
		ids.clear();
		// cleanup systems
		id_manager.reset();
		collision_manager.cleanup();
		movement_manager.cleanup();
		// reset event senders
		collision_sender.clear();
		move_sender.clear();
		teleport_sender.clear();
	}

	core::ObjectID add_object(sf::Vector2u const& pos, bool is_projectile) {
		auto id = id_manager.acquire();
		ids.push_back(id);
		auto& col = collision_manager.acquire(id);
		col.is_projectile = is_projectile;
		col.radius = core::collision_impl::MAX_PROJECTILE_RADIUS;
		auto& mve = movement_manager.acquire(id);
		mve.scene = 1u;
		mve.pos = sf::Vector2f{pos};
		auto& dungeon = dungeon_system[1u];
		dungeon.getCell(pos).entities.push_back(id);
		return id;
	}
};

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(collision_test)

BOOST_AUTO_TEST_CASE(tile_collision_occures_for_void_tiles) {
	core::DungeonCell cell;
	cell.terrain = core::Terrain::Void;
	BOOST_CHECK(core::checkTileCollision(cell));
}

BOOST_AUTO_TEST_CASE(tile_collision_occures_for_wall_tiles) {
	core::DungeonCell cell;
	cell.terrain = core::Terrain::Wall;
	BOOST_CHECK(core::checkTileCollision(cell));
}

BOOST_AUTO_TEST_CASE(tile_collision_does_not_occure_for_floor_tiles) {
	core::DungeonCell cell;
	cell.terrain = core::Terrain::Floor;
	BOOST_CHECK(!core::checkTileCollision(cell));
}

BOOST_AUTO_TEST_CASE(regular_objects_collision_fails_if_bullet_passed_in) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	auto bullet = fix.add_object({1u, 1u}, true);
	auto& c_b = fix.collision_manager.query(bullet);
	auto& dungeon = fix.dungeon_system[1];
	auto& cell = dungeon.getCell({1u, 1u});
	BOOST_CHECK_ASSERT(
		core::checkObjectCollision(fix.collision_manager, cell, c_b));
}

BOOST_AUTO_TEST_CASE(
	regular_object_does_not_collide_if_nobody_to_collide_with) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	auto object = fix.add_object({1u, 1u}, false);
	auto& c_o = fix.collision_manager.query(object);
	auto& dungeon = fix.dungeon_system[1];
	auto& cell = dungeon.getCell({1u, 1u});
	BOOST_CHECK(
		core::checkObjectCollision(fix.collision_manager, cell, c_o).empty());
}

BOOST_AUTO_TEST_CASE(
	regular_object_collides_if_cell_is_shared_with_another_regular_object) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	auto object = fix.add_object({1u, 1u}, false);
	auto other = fix.add_object({1u, 1u}, false);
	auto& c_o = fix.collision_manager.query(object);
	auto& dungeon = fix.dungeon_system[1];
	auto& cell = dungeon.getCell({1u, 1u});
	auto colliders = core::checkObjectCollision(fix.collision_manager, cell, c_o);
	BOOST_CHECK(utils::contains(colliders, other));
}

BOOST_AUTO_TEST_CASE(
	regular_object_does_not_collide_with_object_that_should_be_ignored) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	auto object = fix.add_object({1u, 1u}, false);
	auto other = fix.add_object({1u, 1u}, false);
	auto& c_o = fix.collision_manager.query(object);
	c_o.ignore.push_back(other);
	auto& dungeon = fix.dungeon_system[1];
	auto& cell = dungeon.getCell({1u, 1u});
	BOOST_CHECK(
		core::checkObjectCollision(fix.collision_manager, cell, c_o).empty());
}

BOOST_AUTO_TEST_CASE(
	regular_object_does_not_collide_if_cell_is_only_shared_with_a_bullet) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	auto object = fix.add_object({1u, 1u}, false);
	fix.add_object({1u, 1u}, true);
	auto& c_o = fix.collision_manager.query(object);
	auto& dungeon = fix.dungeon_system[1];
	auto& cell = dungeon.getCell({1u, 1u});
	BOOST_CHECK(
		core::checkObjectCollision(fix.collision_manager, cell, c_o).empty());
}

BOOST_AUTO_TEST_CASE(bullet_collision_fails_if_regular_object_was_passed_in) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	auto object = fix.add_object({1u, 1u}, false);
	auto& c_o = fix.collision_manager.query(object);
	BOOST_CHECK_ASSERT(core::checkBulletCollision(
		fix.collision_manager, fix.movement_manager, fix.dungeon_system, c_o));
}

BOOST_AUTO_TEST_CASE(
	bullet_does_not_collide_if_regular_object_is_too_far_away) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	auto bullet = fix.add_object({1u, 1u}, true);
	auto object = fix.add_object({2u, 1u}, false);
	auto& m_o = fix.movement_manager.query(object);
	m_o.pos.x += 0.1f;
	auto& c_b = fix.collision_manager.query(bullet);
	auto targets = core::checkBulletCollision(
		fix.collision_manager, fix.movement_manager, fix.dungeon_system, c_b);
	BOOST_CHECK(targets.empty());
}

BOOST_AUTO_TEST_CASE(bullet_collides_if_regular_object_at_same_cell) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	auto bullet = fix.add_object({2u, 2u}, true);
	auto object = fix.add_object({2u, 2u}, false);
	auto& b_o = fix.movement_manager.query(bullet);
	b_o.pos.x -= 0.49f;
	auto& m_o = fix.movement_manager.query(object);
	m_o.pos.x += 0.49f;
	auto& c_b = fix.collision_manager.query(bullet);
	auto targets = core::checkBulletCollision(
		fix.collision_manager, fix.movement_manager, fix.dungeon_system, c_b);
	BOOST_REQUIRE_EQUAL(targets.size(), 1u);
	BOOST_CHECK_EQUAL(targets[0], object);
}

BOOST_AUTO_TEST_CASE(bullet_collides_if_regular_object_at_neigbor_cell) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	auto bullet = fix.add_object({1u, 1u}, true);
	auto object = fix.add_object({2u, 1u}, false);
	auto& m_o = fix.movement_manager.query(object);
	m_o.pos.x -= 0.1f;
	auto& c_b = fix.collision_manager.query(bullet);
	auto targets = core::checkBulletCollision(
		fix.collision_manager, fix.movement_manager, fix.dungeon_system, c_b);
	BOOST_REQUIRE_EQUAL(targets.size(), 1u);
	BOOST_CHECK_EQUAL(targets[0], object);
}

BOOST_AUTO_TEST_CASE(bullet_cannot_collide_with_same_object_twice) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	auto bullet = fix.add_object({1u, 1u}, true);
	auto object = fix.add_object({2u, 1u}, false);
	auto& m_o = fix.movement_manager.query(object);
	m_o.pos.x -= 0.1f;
	auto& c_b = fix.collision_manager.query(bullet);
	c_b.ignore.push_back(object);
	auto targets = core::checkBulletCollision(
		fix.collision_manager, fix.movement_manager, fix.dungeon_system, c_b);
	BOOST_CHECK(targets.empty());
}

BOOST_AUTO_TEST_CASE(bullet_collides_with_all_possible_regular_object) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	auto bullet = fix.add_object({1u, 1u}, true);
	auto object = fix.add_object({1u, 1u}, false);
	auto& m_o = fix.movement_manager.query(object);
	m_o.pos.x += 0.35f;
	auto other = fix.add_object({1u, 1u}, false);
	auto& m_o2 = fix.movement_manager.query(other);
	m_o2.pos.x += 0.2f;
	auto last = fix.add_object({1u, 1u}, false);
	auto& m_l = fix.movement_manager.query(last);
	m_l.pos.x += 0.3f;
	auto& c_b = fix.collision_manager.query(bullet);
	auto targets = core::checkBulletCollision(
		fix.collision_manager, fix.movement_manager, fix.dungeon_system, c_b);
	BOOST_REQUIRE_EQUAL(targets.size(), 3u);
	BOOST_CHECK(utils::contains(targets, object));
	BOOST_CHECK(utils::contains(targets, other));
	BOOST_CHECK(utils::contains(targets, last));
}

BOOST_AUTO_TEST_CASE(bullet_does_not_collide_with_other_bullet) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	auto bullet = fix.add_object({1u, 1u}, true);
	fix.add_object({1u, 1u}, true);
	auto& c_b = fix.collision_manager.query(bullet);
	auto targets = core::checkBulletCollision(
		fix.collision_manager, fix.movement_manager, fix.dungeon_system, c_b);
	BOOST_CHECK(targets.empty());
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(regular_objects_tile_collision_is_checked_on_tile_left) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	auto actor = fix.add_object({1u, 1u}, false);
	// move into void
	core::MoveEvent event;
	event.actor = actor;
	event.source = {1u, 1u};
	event.target = {1u, 0u};
	event.type = core::MoveEvent::Left;
	auto& m_a = fix.movement_manager.query(actor);
	m_a.target = event.target;
	// propagate movement
	auto const& f_a = fix.collision_manager.query(actor);
	core::collision_impl::onTileLeft(fix.context, f_a, event);
	// expect tile collision
	auto const& colls = fix.collision_sender.data();
	BOOST_REQUIRE_EQUAL(colls.size(), 1u);
	BOOST_CHECK_EQUAL(colls[0].actor, actor);
	BOOST_CHECK_EQUAL(colls[0].collider, 0u);
	BOOST_CHECK(colls[0].reset);
	BOOST_CHECK_VECTOR_EQUAL(colls[0].pos, event.target);
	BOOST_CHECK_VECTOR_EQUAL(colls[0].reset_to, event.source);
}

BOOST_AUTO_TEST_CASE(regular_objects_object_collision_is_checked_on_tile_left) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	auto actor = fix.add_object({1u, 1u}, false);
	auto other = fix.add_object({2u, 1u}, false);
	// move into void
	core::MoveEvent event;
	event.actor = actor;
	event.source = {1u, 1u};
	event.target = {2u, 1u};
	event.type = core::MoveEvent::Left;
	auto& m_a = fix.movement_manager.query(actor);
	m_a.target = event.target;
	// propagate movement
	auto const& f_a = fix.collision_manager.query(actor);
	core::collision_impl::onTileLeft(fix.context, f_a, event);
	// expect object collision
	auto const& colls = fix.collision_sender.data();
	BOOST_REQUIRE_EQUAL(colls.size(), 1u);
	BOOST_CHECK_EQUAL(colls[0].actor, actor);
	BOOST_CHECK_EQUAL(colls[0].collider, other);
	BOOST_CHECK(colls[0].reset);
	BOOST_CHECK_VECTOR_EQUAL(colls[0].pos, event.target);
	BOOST_CHECK_VECTOR_EQUAL(colls[0].reset_to, event.source);
	// expect no forwarded movement
	BOOST_CHECK(fix.move_sender.data().empty());
}

BOOST_AUTO_TEST_CASE(bullets_tile_collision_is_not_checked_on_tile_left) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	auto actor = fix.add_object({1u, 1u}, true);
	// move into void
	core::MoveEvent event;
	event.actor = actor;
	event.source = {1u, 1u};
	event.target = {1u, 0u};
	event.type = core::MoveEvent::Left;
	auto& m_a = fix.movement_manager.query(actor);
	m_a.target = event.target;
	// propagate movement
	auto const& f_a = fix.collision_manager.query(actor);
	core::collision_impl::onTileLeft(fix.context, f_a, event);
	// expect no tile collision
	auto const& colls = fix.collision_sender.data();
	BOOST_CHECK(colls.empty());
}

BOOST_AUTO_TEST_CASE(bullets_object_collision_is_not_checked_on_tile_left) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	auto actor = fix.add_object({1u, 1u}, true);
	fix.add_object({2u, 1u}, false);
	// move into void
	core::MoveEvent event;
	event.actor = actor;
	event.source = {1u, 1u};
	event.target = {2u, 1u};
	event.type = core::MoveEvent::Left;
	auto& m_a = fix.movement_manager.query(actor);
	m_a.target = event.target;
	// propagate movement
	auto const& f_a = fix.collision_manager.query(actor);
	core::collision_impl::onTileLeft(fix.context, f_a, event);
	// expect no collision
	auto const& colls = fix.collision_sender.data();
	BOOST_REQUIRE(colls.empty());
}

BOOST_AUTO_TEST_CASE(bullets_tile_collision_is_checked_on_tile_reached) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	auto actor = fix.add_object({1u, 1u}, true);
	// move into void
	core::MoveEvent event;
	event.actor = actor;
	event.source = {1u, 1u};
	event.target = {1u, 0u};
	event.type = core::MoveEvent::Reached;
	auto& m_a = fix.movement_manager.query(actor);
	m_a.target = event.target;
	m_a.pos = sf::Vector2f{event.target};
	// propagate movement
	auto const& f_a = fix.collision_manager.query(actor);
	core::collision_impl::onTileReached(fix.context, f_a, event);
	// expect tile collision
	auto const& colls = fix.collision_sender.data();
	BOOST_REQUIRE_EQUAL(colls.size(), 1u);
	BOOST_CHECK_EQUAL(colls[0].actor, actor);
	BOOST_CHECK_EQUAL(colls[0].collider, 0u);
	BOOST_CHECK(colls[0].reset);
	BOOST_CHECK_VECTOR_EQUAL(colls[0].pos, event.target);
	BOOST_CHECK_VECTOR_EQUAL(colls[0].reset_to, event.target);
}

BOOST_AUTO_TEST_CASE(bullets_object_collision_is_checked_on_bullet_check) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	auto actor = fix.add_object({1u, 1u}, true);
	auto other = fix.add_object({2u, 1u}, false);
	// move bullet close to other object
	auto& m_a = fix.movement_manager.query(actor);
	m_a.pos.x += 0.75f;
	// trigger bullet's collision check
	auto& f_a = fix.collision_manager.query(actor);
	core::collision_impl::onBulletCheck(fix.context, f_a);
	// expect object collision
	auto const& colls = fix.collision_sender.data();
	BOOST_REQUIRE_EQUAL(colls.size(), 1u);
	BOOST_CHECK_EQUAL(colls[0].actor, actor);
	BOOST_CHECK_EQUAL(colls[0].collider, other);
	BOOST_CHECK(!colls[0].reset);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(leaving_tile_is_not_forwarded_if_a_collision_happened) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	auto actor = fix.add_object({1u, 1u}, false);
	// move into void
	core::MoveEvent event;
	event.actor = actor;
	event.source = {1u, 1u};
	event.target = {1u, 0u};
	event.type = core::MoveEvent::Left;
	auto& m_a = fix.movement_manager.query(actor);
	m_a.target = event.target;
	// propagate movement
	auto const& f_a = fix.collision_manager.query(actor);
	core::collision_impl::onTileLeft(fix.context, f_a, event);
	// expect collision event
	auto const& colls = fix.collision_sender.data();
	BOOST_REQUIRE_EQUAL(colls.size(), 1u);
	// expect movement not being forwarded
	auto const& moves = fix.move_sender.data();
	BOOST_CHECK(moves.empty());
}

BOOST_AUTO_TEST_CASE(leaving_tile_is_forwarded_if_a_no_collision_happened) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	auto actor = fix.add_object({1u, 1u}, false);
	// move into floor
	core::MoveEvent event;
	event.actor = actor;
	event.source = {1u, 1u};
	event.target = {1u, 2u};
	event.type = core::MoveEvent::Left;
	auto& m_a = fix.movement_manager.query(actor);
	m_a.target = event.target;
	// propagate movement
	auto const& f_a = fix.collision_manager.query(actor);
	core::collision_impl::onTileLeft(fix.context, f_a, event);
	// expect no collision event
	auto const& colls = fix.collision_sender.data();
	BOOST_CHECK(colls.empty());
	// expect movement being forwarded
	auto const& moves = fix.move_sender.data();
	BOOST_REQUIRE_EQUAL(moves.size(), 1u);
	BOOST_CHECK_EQUAL(moves[0].actor, event.actor);
	BOOST_CHECK_VECTOR_EQUAL(moves[0].source, event.source);
	BOOST_CHECK_VECTOR_EQUAL(moves[0].target, event.target);
	BOOST_CHECK_EQUAL(moves[0].type, event.type);
}

BOOST_AUTO_TEST_CASE(reaching_tile_is_not_forwarded_if_a_collision_happened) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	auto actor = fix.add_object({1u, 1u}, true);
	// move into void
	core::MoveEvent event;
	event.actor = actor;
	event.source = {1u, 1u};
	event.target = {1u, 0u};
	event.type = core::MoveEvent::Reached;
	auto& m_a = fix.movement_manager.query(actor);
	m_a.target = event.target;
	m_a.pos = sf::Vector2f{event.target};
	// propagate movement
	auto const& f_a = fix.collision_manager.query(actor);
	core::collision_impl::onTileReached(fix.context, f_a, event);
	// expect collision event
	auto const& colls = fix.collision_sender.data();
	BOOST_REQUIRE_EQUAL(colls.size(), 1u);
	// expect movement not being forwarded
	auto const& moves = fix.move_sender.data();
	BOOST_CHECK(moves.empty());
}

BOOST_AUTO_TEST_CASE(reaching_tile_is_forwarded_if_a_no_collision_happened) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	auto actor = fix.add_object({1u, 1u}, false);
	// move into floor
	core::MoveEvent event;
	event.actor = actor;
	event.source = {1u, 1u};
	event.target = {1u, 2u};
	event.type = core::MoveEvent::Reached;
	auto& m_a = fix.movement_manager.query(actor);
	m_a.target = event.target;
	m_a.pos = sf::Vector2f{event.target};
	// propagate movement
	auto const& f_a = fix.collision_manager.query(actor);
	core::collision_impl::onTileReached(fix.context, f_a, event);
	// expect no collision event
	auto const& colls = fix.collision_sender.data();
	BOOST_CHECK(colls.empty());
	// expect movement being forwarded
	auto const& moves = fix.move_sender.data();
	BOOST_REQUIRE_EQUAL(moves.size(), 1u);
	BOOST_CHECK_EQUAL(moves[0].actor, event.actor);
	BOOST_CHECK_VECTOR_EQUAL(moves[0].source, event.source);
	BOOST_CHECK_VECTOR_EQUAL(moves[0].target, event.target);
	BOOST_CHECK_EQUAL(moves[0].type, event.type);
}

BOOST_AUTO_TEST_SUITE_END()

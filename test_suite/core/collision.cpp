#include <vector>
#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>
#include <testsuite/singleton.hpp>

#include <utils/assert.hpp>
#include <utils/algorithm.hpp>
#include <core/collision.hpp>

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
				cell.terrain = core::Terrain::Floor;
				cell.trigger = nullptr;
			}
		}
		// remove components
		for (auto id : ids) {
			collision_manager.tryRelease(id);
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
		
		// clear logs
		log.debug.clear();
		log.warning.clear();
		log.error.clear();
	}

	core::ObjectID add_object(sf::Vector2u const& pos, utils::Collider* shape=nullptr) {
		auto id = id_manager.acquire();
		ids.push_back(id);
		if (shape != nullptr) {
			auto& col = collision_manager.acquire(id);
			col.is_projectile = false;
			col.shape = *shape; // copy shape
		}
		auto& mve = movement_manager.acquire(id);
		mve.scene = 1u;
		mve.pos = sf::Vector2f{pos};
		mve.last_pos = mve.pos;
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

// ----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(object_collision_stops_if_actor_cannot_collide) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	utils::Collider shape;

	auto actor = fix.add_object({1u, 1u});
	auto target = fix.add_object({1u, 1u}, &shape);
	
	auto const & actor_move  = fix.movement_manager.query(actor);
	auto const & target_move = fix.movement_manager.query(target);
	
	auto found = core::checkObjectCollision(fix.collision_manager, actor, actor_move.pos, target, target_move.pos);
	BOOST_CHECK(!found);
}

BOOST_AUTO_TEST_CASE(object_collision_stops_if_target_cannot_collide) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	utils::Collider shape;

	auto actor = fix.add_object({1u, 1u}, &shape);
	auto target = fix.add_object({1u, 1u});
	
	auto const & actor_move  = fix.movement_manager.query(actor);
	auto const & target_move = fix.movement_manager.query(target);
	
	auto found = core::checkObjectCollision(fix.collision_manager, actor, actor_move.pos, target, target_move.pos);
	BOOST_CHECK(!found);
}

BOOST_AUTO_TEST_CASE(object_collision_detects_circ_circ_collision) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	utils::Collider shape;
	shape.radius  = 0.5f;
	shape.is_aabb = false;

	auto actor = fix.add_object({1u, 1u}, &shape);
	auto target = fix.add_object({1u, 1u}, &shape);
	
	auto const & actor_move  = fix.movement_manager.query(actor);
	auto const & target_move = fix.movement_manager.query(target);
	
	auto found = core::checkObjectCollision(fix.collision_manager, actor, actor_move.pos, target, target_move.pos);
	BOOST_CHECK(found);
}

BOOST_AUTO_TEST_CASE(object_collision_detects_circ_aabb_collision) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	utils::Collider shape1, shape2;
	shape1.radius  = 0.5f;
	shape1.is_aabb = false;
	shape2.size    = {0.5f, 0.5f};
	shape2.is_aabb = true;
	shape2.updateRadiusAABB(); // <-- update broadphase radius using AABB size

	auto actor = fix.add_object({1u, 1u}, &shape1);
	auto target = fix.add_object({1u, 1u}, &shape2);
	
	auto const & actor_move  = fix.movement_manager.query(actor);
	auto const & target_move = fix.movement_manager.query(target);
	
	auto found = core::checkObjectCollision(fix.collision_manager, actor, actor_move.pos, target, target_move.pos);
	BOOST_CHECK(found);
}

BOOST_AUTO_TEST_CASE(object_collision_detects_aabb_circ_collision) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	utils::Collider shape1, shape2;
	shape1.radius  = 0.5f;
	shape1.is_aabb = false;
	shape2.size    = {0.5f, 0.5f};
	shape2.is_aabb = true;
	shape2.updateRadiusAABB(); // <-- update broadphase radius using AABB size

	auto actor = fix.add_object({1u, 1u}, &shape2);
	auto target = fix.add_object({1u, 1u}, &shape1);
	
	auto const & actor_move  = fix.movement_manager.query(actor);
	auto const & target_move = fix.movement_manager.query(target);
	
	auto found = core::checkObjectCollision(fix.collision_manager, actor, actor_move.pos, target, target_move.pos);
	BOOST_CHECK(found);
}

BOOST_AUTO_TEST_CASE(object_collision_detects_aabb_aabb_collision) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	utils::Collider shape;
	shape.size    = {0.5f, 0.5f};
	shape.is_aabb = true;
	shape.updateRadiusAABB(); // <-- update broadphase radius using AABB size

	auto actor = fix.add_object({1u, 1u}, &shape);
	auto target = fix.add_object({1u, 1u}, &shape);
	
	auto const & actor_move  = fix.movement_manager.query(actor);
	auto const & target_move = fix.movement_manager.query(target);
	
	auto found = core::checkObjectCollision(fix.collision_manager, actor, actor_move.pos, target, target_move.pos);
	BOOST_CHECK(found);
}

BOOST_AUTO_TEST_CASE(object_collision_detects_no_collision_if_too_far) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	utils::Collider shape;
	shape.radius  = 0.9f;
	shape.is_aabb = false;

	auto actor = fix.add_object({1u, 1u}, &shape);
	auto target = fix.add_object({3u, 1u}, &shape);
	
	auto const & actor_move  = fix.movement_manager.query(actor);
	auto const & target_move = fix.movement_manager.query(target);
	
	auto found = core::checkObjectCollision(fix.collision_manager, actor, actor_move.pos, target, target_move.pos);
	BOOST_CHECK(!found);
}

BOOST_AUTO_TEST_CASE(object_collision_detects_no_collision_if_on_ignore) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	utils::Collider shape;
	shape.radius  = 1.f;
	shape.is_aabb = false;

	auto actor = fix.add_object({1u, 1u}, &shape);
	auto target = fix.add_object({3u, 1u}, &shape);
	
	auto& actor_coll = fix.collision_manager.query(actor);
	actor_coll.ignore.push_back(target);
	
	auto const & actor_move  = fix.movement_manager.query(actor);
	auto const & target_move = fix.movement_manager.query(target);
	
	auto found = core::checkObjectCollision(fix.collision_manager, actor, actor_move.pos, target, target_move.pos);
	BOOST_CHECK(!found);
}

// ----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(regular_object_collision_detects_one_object_collision) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	utils::Collider shape;
	shape.radius  = 1.f;
	shape.is_aabb = false;

	auto actor = fix.add_object({1u, 1u}, &shape);
	auto target = fix.add_object({2u, 1u}, &shape);
	auto other = fix.add_object({1u, 2u}, &shape);
	
	auto const & actor_move = fix.movement_manager.query(actor);
	
	core::CollisionResult result;
	core::collision_impl::checkAnyCollision(fix.context, actor_move, result);
	BOOST_REQUIRE(result.meansCollision());
	BOOST_CHECK(result.interrupt);
	BOOST_REQUIRE_EQUAL(result.objects.size(), 1u);
	BOOST_CHECK(utils::contains(result.objects, target) || utils::contains(result.objects, other));
}

BOOST_AUTO_TEST_CASE(regular_object_cannot_collide_with_projectile) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	utils::Collider shape;
	shape.radius  = 1.f;
	shape.is_aabb = false;

	auto actor = fix.add_object({1u, 1u}, &shape);
	auto target = fix.add_object({2u, 1u}, &shape);
	
	fix.collision_manager.query(target).is_projectile = true;
	
	auto const & actor_move = fix.movement_manager.query(actor);
	
	core::CollisionResult result;
	core::collision_impl::checkAnyCollision(fix.context, actor_move, result);
	BOOST_REQUIRE(!result.meansCollision());
}

BOOST_AUTO_TEST_CASE(projectile_can_collide_with_regular_object) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	utils::Collider shape;
	shape.radius  = 1.f;
	shape.is_aabb = false;

	auto actor = fix.add_object({1u, 1u}, &shape);
	auto target = fix.add_object({2u, 1u}, &shape);
	
	fix.collision_manager.query(target).is_projectile = true;
	
	auto const & target_move = fix.movement_manager.query(target);
	
	core::CollisionResult result;
	core::collision_impl::checkAnyCollision(fix.context, target_move, result);
	BOOST_REQUIRE(result.meansCollision());
}

BOOST_AUTO_TEST_CASE(projectiles_can_collide_with_each_other) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	utils::Collider shape;
	shape.radius  = 1.f;
	shape.is_aabb = false;

	auto actor = fix.add_object({1u, 1u}, &shape);
	auto target = fix.add_object({2u, 1u}, &shape);
	
	fix.collision_manager.query(actor).is_projectile = true;
	fix.collision_manager.query(target).is_projectile = true;
	
	auto const & actor_move = fix.movement_manager.query(actor);
	
	core::CollisionResult result;
	core::collision_impl::checkAnyCollision(fix.context, actor_move, result);
	BOOST_REQUIRE(result.meansCollision());
}

BOOST_AUTO_TEST_CASE(regular_object_collision_detects_tile_collision_but_no_object_collisions) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	utils::Collider shape;
	shape.radius  = 1.f;
	shape.is_aabb = false;

	auto actor = fix.add_object({1u, 1u}, &shape);
	auto target = fix.add_object({2u, 1u}, &shape);
	auto other = fix.add_object({1u, 2u}, &shape);
	
	auto const & actor_move = fix.movement_manager.query(actor);
	auto& cell = fix.dungeon_system[1u].getCell(sf::Vector2u{actor_move.pos});
	cell.terrain = core::Terrain::Wall;
	
	core::CollisionResult result;
	core::collision_impl::checkAnyCollision(fix.context, actor_move, result);
	BOOST_REQUIRE(result.meansCollision());
	BOOST_CHECK(result.interrupt);
	BOOST_REQUIRE_EQUAL(result.objects.size(), 0u);
}

BOOST_AUTO_TEST_CASE(collision_with_any_object_does_interrupt) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	utils::Collider shape;
	shape.radius  = 1.f;
	shape.is_aabb = false;

	auto actor = fix.add_object({1u, 1u}, &shape);
	auto target = fix.add_object({2u, 1u}, &shape);
	auto other = fix.add_object({1u, 2u}, &shape);
	
	auto const & actor_move = fix.movement_manager.query(actor);
	auto& target_coll = fix.collision_manager.query(target);
	target_coll.is_projectile = true;
	auto& cell = fix.dungeon_system[1u].getCell(sf::Vector2u{actor_move.pos});
	cell.terrain = core::Terrain::Wall;
	
	core::CollisionResult result;
	core::collision_impl::checkAnyCollision(fix.context, actor_move, result);
	BOOST_REQUIRE(result.meansCollision());
	BOOST_CHECK(result.interrupt);
	BOOST_REQUIRE_EQUAL(result.objects.size(), 0u);
}

BOOST_AUTO_TEST_CASE(projectile_object_collision_detects_one_object_collision) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	utils::Collider shape;
	shape.radius  = 1.f;
	shape.is_aabb = false;

	auto actor = fix.add_object({1u, 1u}, &shape);
	auto target = fix.add_object({2u, 1u}, &shape);
	auto other = fix.add_object({1u, 2u}, &shape);
	
	auto const & actor_move  = fix.movement_manager.query(actor);
	auto& actor_coll = fix.collision_manager.query(actor);
	actor_coll.is_projectile = true;
	
	core::CollisionResult result;
	core::collision_impl::checkAnyCollision(fix.context, actor_move, result);
	BOOST_REQUIRE(result.meansCollision());
	BOOST_REQUIRE_EQUAL(result.objects.size(), 2u);
	BOOST_CHECK(utils::contains(result.objects, target));
	BOOST_CHECK(utils::contains(result.objects, other));
}

BOOST_AUTO_TEST_CASE(projectile_object_collision_detects_tile_collision_but_and_all_object_collisions) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	utils::Collider shape;
	shape.radius  = 1.f;
	shape.is_aabb = false;

	auto actor = fix.add_object({1u, 1u}, &shape);
	auto target = fix.add_object({2u, 1u}, &shape);
	auto other = fix.add_object({1u, 2u}, &shape);
	
	auto const & actor_move = fix.movement_manager.query(actor);
	auto& actor_coll = fix.collision_manager.query(actor);
	actor_coll.is_projectile = true;
	auto& cell = fix.dungeon_system[1u].getCell(sf::Vector2u{actor_move.pos});
	cell.terrain = core::Terrain::Wall;
	
	core::CollisionResult result;
	core::collision_impl::checkAnyCollision(fix.context, actor_move, result);
	BOOST_REQUIRE(result.meansCollision());
	BOOST_REQUIRE_EQUAL(result.objects.size(), 2u);
	BOOST_CHECK(utils::contains(result.objects, target));
	BOOST_CHECK(utils::contains(result.objects, other));
}

// ----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(update_collision_map_works_for_remaining_in_cell) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	utils::Collider shape;
	shape.radius  = 1.f;
	shape.is_aabb = false;

	auto actor = fix.add_object({1u, 1u}, &shape);
	auto& actor_move = fix.movement_manager.query(actor);
	actor_move.last_pos = actor_move.pos;
	// slighly change it
	actor_move.pos.x += 0.2f;
	actor_move.pos.y += 0.3f;
	auto& src_cell = fix.dungeon_system[1u].getCell(sf::Vector2u{actor_move.last_pos});
	auto& dst_cell = fix.dungeon_system[1u].getCell(sf::Vector2u{actor_move.pos});
	
	core::collision_impl::updateCollisionMap(fix.context, actor_move);
	BOOST_REQUIRE_EQUAL(&src_cell, &dst_cell);
	BOOST_CHECK(utils::contains(src_cell.entities, actor));
}

BOOST_AUTO_TEST_CASE(update_collision_map_works_for_moving_between_cells) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	utils::Collider shape;
	shape.radius  = 1.f;
	shape.is_aabb = false;

	auto actor = fix.add_object({1u, 1u}, &shape);
	auto& actor_move = fix.movement_manager.query(actor);
	actor_move.last_pos = actor_move.pos;
	// slighly change it
	actor_move.pos.x -= 0.2f;
	actor_move.pos.y += 0.3f;
	auto& src_cell = fix.dungeon_system[1u].getCell(sf::Vector2u{actor_move.last_pos});
	auto& dst_cell = fix.dungeon_system[1u].getCell(sf::Vector2u{actor_move.pos});
	
	BOOST_REQUIRE(utils::contains(src_cell.entities, actor));
	BOOST_REQUIRE(!utils::contains(dst_cell.entities, actor));
	
	core::collision_impl::updateCollisionMap(fix.context, actor_move);
	BOOST_CHECK(!utils::contains(src_cell.entities, actor));
	BOOST_CHECK(utils::contains(dst_cell.entities, actor));
}

// ----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(regular_tile_collision_is_propagated_on_movement) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	utils::Collider shape;
	shape.radius  = 1.f;
	shape.is_aabb = false;

	auto actor = fix.add_object({1u, 1u}, &shape);
	auto target = fix.add_object({2u, 1u}, &shape);
	
	auto& actor_move = fix.movement_manager.query(actor);
	actor_move.pos = {2.03f, 2.17f};
	actor_move.is_moving = true;
	auto& cell = fix.dungeon_system[1u].getCell({2u, 2u});
	cell.terrain = core::Terrain::Wall;
	auto& data = fix.collision_sender.data();
	
	core::collision_impl::checkAllCollisions(fix.context);
	BOOST_REQUIRE_EQUAL(data.size(), 1u);
	BOOST_CHECK_EQUAL(data[0].actor, actor);
	BOOST_CHECK_EQUAL(data[0].collider, 0);
}

BOOST_AUTO_TEST_CASE(regular_object_collision_is_propagated_on_movement) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	utils::Collider shape;
	shape.radius  = 1.f;
	shape.is_aabb = false;

	auto actor = fix.add_object({1u, 1u}, &shape);
	auto target = fix.add_object({2u, 1u}, &shape);
	auto other = fix.add_object({1u, 2u}, &shape);
	
	auto& actor_move = fix.movement_manager.query(actor);
	actor_move.pos = {2.03f, 2.17f};
	actor_move.is_moving = true;
	auto& data = fix.collision_sender.data();
	
	core::collision_impl::checkAllCollisions(fix.context);
	BOOST_REQUIRE_EQUAL(data.size(), 1u);
	BOOST_CHECK_EQUAL(data[0].actor, actor);
	BOOST_CHECK((data[0].collider == target) || (data[0].collider == other));
}

BOOST_AUTO_TEST_CASE(no_regular_collision_propagated_without_movement) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	utils::Collider shape;
	shape.radius  = 1.f;
	shape.is_aabb = false;

	auto actor = fix.add_object({1u, 1u}, &shape);
	auto target = fix.add_object({2u, 1u}, &shape);
	
	auto& actor_move = fix.movement_manager.query(actor);
	actor_move.pos = {2.03f, 2.17f};
	actor_move.is_moving = false;
	auto& cell = fix.dungeon_system[1u].getCell({1u, 1u});
	cell.terrain = core::Terrain::Wall;
	auto& data = fix.collision_sender.data();
	
	core::collision_impl::checkAllCollisions(fix.context);
	BOOST_CHECK_EQUAL(data.size(), 0u);
}

BOOST_AUTO_TEST_CASE(projectile_tile_collision_is_propagated_on_movement) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	utils::Collider shape;
	shape.radius  = 1.f;
	shape.is_aabb = false;

	auto actor = fix.add_object({1u, 1u}, &shape);
	
	auto& actor_move = fix.movement_manager.query(actor);
	actor_move.pos = {2.03f, 2.17f};
	actor_move.is_moving = true;
	auto& actor_coll = fix.collision_manager.query(actor);
	actor_coll.is_projectile = true;
	auto& cell = fix.dungeon_system[1u].getCell(sf::Vector2u{actor_move.pos});
	cell.terrain = core::Terrain::Wall;
	auto& data = fix.collision_sender.data();
	
	core::collision_impl::checkAllCollisions(fix.context);
	BOOST_REQUIRE_EQUAL(data.size(), 1u);
	BOOST_CHECK_EQUAL(data[0].actor, actor);
	BOOST_CHECK_EQUAL(data[0].collider, 0);
}

BOOST_AUTO_TEST_CASE(projectile_object_collisions_are_propagated_on_movement) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	utils::Collider shape;
	shape.radius  = 1.f;
	shape.is_aabb = false;

	auto actor = fix.add_object({1u, 1u}, &shape);
	auto target = fix.add_object({2u, 1u}, &shape);
	auto other = fix.add_object({1u, 2u}, &shape);
	
	auto& actor_move = fix.movement_manager.query(actor);
	actor_move.pos = {2.03f, 2.17f};
	actor_move.is_moving = true;
	auto& actor_coll = fix.collision_manager.query(actor);
	actor_coll.is_projectile = true;
	auto& data = fix.collision_sender.data();
	
	core::collision_impl::checkAllCollisions(fix.context);
	BOOST_REQUIRE_EQUAL(data.size(), 2u);
	BOOST_CHECK_EQUAL(data[0].actor, actor);
	BOOST_CHECK_EQUAL(data[0].collider, target);
	BOOST_CHECK_EQUAL(data[1].actor, actor);
	BOOST_CHECK_EQUAL(data[1].collider, other);
}

BOOST_AUTO_TEST_CASE(projectile_object_collisions_updates_collisionmap) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	utils::Collider shape;
	shape.radius  = 1.f;
	shape.is_aabb = false;

	auto actor = fix.add_object({1u, 1u}, &shape);
	auto target = fix.add_object({2u, 1u}, &shape);
	auto other = fix.add_object({1u, 2u}, &shape);
	
	auto& actor_move = fix.movement_manager.query(actor);
	actor_move.pos = {2.03f, 2.17f};
	actor_move.is_moving = true;
	auto& actor_coll = fix.collision_manager.query(actor);
	actor_coll.is_projectile = true;
	auto& data = fix.collision_sender.data();
	
	core::collision_impl::checkAllCollisions(fix.context);
	BOOST_REQUIRE_EQUAL(data.size(), 2u); // object collision occured!
	
	auto const & cell = fix.dungeon_system[1u].getCell(sf::Vector2u{actor_move.pos});
	auto updated = utils::contains(cell.entities, actor);
	BOOST_CHECK(updated);
}

BOOST_AUTO_TEST_CASE(no_projectile_collisions_are_propagated_without_movement) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	utils::Collider shape;
	shape.radius  = 1.f;
	shape.is_aabb = false;

	auto actor = fix.add_object({1u, 1u}, &shape);
	auto target = fix.add_object({2u, 1u}, &shape);
	auto other = fix.add_object({1u, 2u}, &shape);
	
	auto& actor_move = fix.movement_manager.query(actor);
	actor_move.pos = {2.03f, 2.17f};
	actor_move.is_moving = false;
	auto& actor_coll = fix.collision_manager.query(actor);
	actor_coll.is_projectile = true;
	auto& data = fix.collision_sender.data();
	
	core::collision_impl::checkAllCollisions(fix.context);
	BOOST_CHECK_EQUAL(data.size(), 0u);
}

BOOST_AUTO_TEST_CASE(projectiles_only_collide_once_with_each_object) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	utils::Collider shape;
	shape.radius  = 1.f;
	shape.is_aabb = false;

	auto actor = fix.add_object({1u, 1u}, &shape);
	auto target = fix.add_object({2u, 1u}, &shape);
	auto other = fix.add_object({1u, 2u}, &shape);
	
	auto& actor_move = fix.movement_manager.query(actor);
	actor_move.is_moving = true;
	auto& actor_coll = fix.collision_manager.query(actor);
	actor_coll.is_projectile = true;
	auto& data = fix.collision_sender.data();
	
	core::collision_impl::checkAllCollisions(fix.context);
	BOOST_CHECK_EQUAL(data.size(), 2u);
	BOOST_REQUIRE_EQUAL(actor_coll.ignore.size(), 2u);
	BOOST_CHECK(utils::contains(actor_coll.ignore, target));
	BOOST_CHECK(utils::contains(actor_coll.ignore, other));
	
	fix.collision_sender.clear();
	core::collision_impl::checkAllCollisions(fix.context);
	BOOST_CHECK_EQUAL(data.size(), 0u);
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(reaching_tile_executes_and_expires_trigger) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	utils::Collider shape;

	auto& trigger = fix.dungeon_system[1u].getCell({1u, 2u}).trigger;
	trigger = std::make_unique<DemoTrigger>();

	auto actor = fix.add_object({1u, 1u}, &shape);
	auto& actor_move = fix.movement_manager.query(actor);
	actor_move.is_moving = true;
	actor_move.pos = {1.f, 2.f};
	
	core::collision_impl::checkAllCollisions(fix.context);

	// expect deleted trigger
	BOOST_CHECK(trigger == nullptr);
}

BOOST_AUTO_TEST_CASE(collision_system_can_handle_entity_without_collisiondata) {
	auto& fix = Singleton<CollisionFixture>::get();
	fix.reset();

	auto actor = fix.add_object({1u, 1u}, nullptr);
	auto& actor_move = fix.movement_manager.query(actor);
	actor_move.is_moving = true;
	actor_move.pos = {1.f, 2.f};
	
	BOOST_CHECK_NO_ASSERT(core::collision_impl::checkAllCollisions(fix.context));
}

BOOST_AUTO_TEST_SUITE_END()

#include <vector>
#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>
#include <testsuite/singleton.hpp>

#include <core/teleport.hpp>

struct TeleportFixture {
	sf::Texture dummy_tileset;

	core::IdManager id_manager;
	core::MovementManager movement;
	core::CollisionManager collision;
	core::DungeonSystem dungeon;
	std::vector<core::ObjectID> ids;

	TeleportFixture()
		: dummy_tileset{}
		, id_manager{}
		, movement{}
		, collision{}
		, dungeon{} {
		// add scenes
		for (auto i = 0u; i < 2; ++i) {
			auto scene = dungeon.create(
				dummy_tileset, sf::Vector2u{12u, 10u}, sf::Vector2f{1.f, 1.f});
			assert(scene == 1u + i);
		}
	}

	void reset() {
		for (auto i = 0u; i < 2; ++i) {
			auto& d = dungeon[1u + i];
			for (auto y = 0u; y < 10u; ++y) {
				for (auto x = 0u; x < 12u; ++x) {
					auto& cell = d.getCell({x, y});
					if (x == 0u || y == 0u) {
						cell.terrain = core::Terrain::Wall;
					} else {
						cell.terrain = core::Terrain::Floor;
					}
					cell.entities.clear();
				}
			}
		}
		for (auto id : ids) {
			movement.release(id);
			collision.release(id);
		}
		ids.clear();
		movement.cleanup();
		collision.cleanup();
		id_manager.reset();
	}

	core::MovementData& add_object() {
		auto id = id_manager.acquire();
		ids.push_back(id);
		auto& c = collision.acquire(id);
		c.shape.radius = 0.49f;
		return movement.acquire(id);
	}
};

BOOST_AUTO_TEST_SUITE(teleport_test)

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(cannot_spawn_if_object_has_already_spawned_somewhere) {
	auto& fix = Singleton<TeleportFixture>::get();
	fix.reset();

	auto& data = fix.add_object();
	data.scene = 5u;
	auto& dungeon = fix.dungeon[1u];
	BOOST_CHECK_ASSERT(core::spawn(dungeon, data, {1u, 1u}));
}

BOOST_AUTO_TEST_CASE(can_spawn_if_object_has_not_spawned_somewhereyed) {
	auto& fix = Singleton<TeleportFixture>::get();
	fix.reset();

	auto& data = fix.add_object();
	data.scene = 0u;
	auto& dungeon = fix.dungeon[1u];
	BOOST_CHECK_NO_ASSERT(core::spawn(dungeon, data, {1u, 2u}));
	BOOST_CHECK_VECTOR_EQUAL(data.pos, sf::Vector2u(1u, 2u));
	BOOST_CHECK_VECTOR_EQUAL(data.target, data.pos);
	BOOST_CHECK_EQUAL(data.scene, 1u);
	auto const& cell = dungeon.getCell({1u, 2u});
	BOOST_CHECK(utils::contains(cell.entities, data.id));
}

BOOST_AUTO_TEST_CASE(spawn_object_sets_dirtyflag) {
	auto& fix = Singleton<TeleportFixture>::get();
	fix.reset();

	auto& data = fix.add_object();
	data.scene = 0u;
	data.has_changed = false;
	auto& dungeon = fix.dungeon[1u];
	BOOST_CHECK_NO_ASSERT(core::spawn(dungeon, data, {1u, 2u}));
	BOOST_CHECK(data.has_changed);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(
	cannot_vanish_if_object_has_not_spawned_at_this_dungeon_yet) {
	auto& fix = Singleton<TeleportFixture>::get();
	fix.reset();

	auto& data = fix.add_object();
	data.scene = 5u;
	auto& dungeon = fix.dungeon[1u];
	BOOST_CHECK_ASSERT(core::vanish(dungeon, data));
}

BOOST_AUTO_TEST_CASE(cannot_vanish_if_object_is_not_located_at_its_cell) {
	auto& fix = Singleton<TeleportFixture>::get();
	fix.reset();

	auto& data = fix.add_object();
	data.scene = 0u;
	auto& dungeon = fix.dungeon[1u];
	core::spawn(dungeon, data, {1u, 2u});
	data.pos = {1.f, 3.f};
	data.target = {1u, 4u};
	BOOST_CHECK_ASSERT(core::vanish(dungeon, data));
}

BOOST_AUTO_TEST_CASE(can_vanish_if_object_it_is_located_there) {
	auto& fix = Singleton<TeleportFixture>::get();
	fix.reset();

	auto& data = fix.add_object();
	data.scene = 0u;
	auto& dungeon = fix.dungeon[1u];
	core::spawn(dungeon, data, {1u, 2u});
	BOOST_CHECK_NO_ASSERT(core::vanish(dungeon, data));
}

BOOST_AUTO_TEST_CASE(vanish_sets_dirtyflag) {
	auto& fix = Singleton<TeleportFixture>::get();
	fix.reset();

	auto& data = fix.add_object();
	data.scene = 0u;
	auto& dungeon = fix.dungeon[1u];
	core::spawn(dungeon, data, {1u, 2u});
	data.has_changed = false;
	BOOST_CHECK_NO_ASSERT(core::vanish(dungeon, data));
	BOOST_CHECK(data.has_changed);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(given_position_can_be_detected_as_free_position) {
	auto& fix = Singleton<TeleportFixture>::get();
	fix.reset();

	auto& data = fix.add_object();
	auto& coll = fix.collision.query(data.id);
	auto& dungeon = fix.dungeon[1u];
	sf::Vector2u pos{1, 1};
	core::SpawnHelper helper{fix.collision, fix.movement, dungeon, data.id};
	auto success = core::getFreePosition(helper, pos);
	BOOST_CHECK(success);
	BOOST_CHECK_VECTOR_EQUAL(pos, sf::Vector2u(1, 1));
}

BOOST_AUTO_TEST_CASE(wall_tile_is_avoided) {
	auto& fix = Singleton<TeleportFixture>::get();
	fix.reset();

	auto& data = fix.add_object();
	auto& coll = fix.collision.query(data.id);
	auto& dungeon = fix.dungeon[1u];
	sf::Vector2u pos{3, 3};
	dungeon.getCell(pos).terrain = core::Terrain::Wall;
	core::SpawnHelper helper{fix.collision, fix.movement, dungeon, data.id};	
	BOOST_CHECK(!core::getFreePosition(helper, pos));
	BOOST_CHECK(core::getFreePosition(helper, pos, 1));
	BOOST_CHECK_VECTOR_EQUAL(pos, sf::Vector2u(2, 2));
}

BOOST_AUTO_TEST_CASE(void_tile_is_avoided) {
	auto& fix = Singleton<TeleportFixture>::get();
	fix.reset();

	auto& data = fix.add_object();
	auto& coll = fix.collision.query(data.id);
	auto& dungeon = fix.dungeon[1u];
	sf::Vector2u pos{3, 3};
	dungeon.getCell(pos).terrain = core::Terrain::Void;
	core::SpawnHelper helper{fix.collision, fix.movement, dungeon, data.id};	
	BOOST_CHECK(!core::getFreePosition(helper, pos));
	BOOST_CHECK(core::getFreePosition(helper, pos, 1));
	BOOST_CHECK_VECTOR_EQUAL(pos, sf::Vector2u(2, 2));
}

BOOST_AUTO_TEST_CASE(object_is_avoided) {
	auto& fix = Singleton<TeleportFixture>::get();
	fix.reset();

	auto& data = fix.add_object();
	auto& coll = fix.collision.query(data.id);
	auto& dungeon = fix.dungeon[1u];
	auto& block = fix.add_object();
	core::spawn(dungeon, block, {1u, 1u});
	sf::Vector2u pos{1, 1};
	core::SpawnHelper helper{fix.collision, fix.movement, dungeon, data.id};
	BOOST_CHECK(!core::getFreePosition(helper, pos));
	BOOST_CHECK(core::getFreePosition(helper, pos, 1));
	BOOST_CHECK_VECTOR_EQUAL(pos, sf::Vector2u(2, 1));
}

BOOST_AUTO_TEST_CASE(finding_free_pos_can_fail) {
	auto& fix = Singleton<TeleportFixture>::get();
	fix.reset();

	// modify terrain
	auto& dungeon = fix.dungeon[1u];
	for (auto y = 0u; y < 10u; ++y) {
		for (auto x = 0u; x < 12u; ++x) {
			dungeon.getCell({x, y}).terrain = core::Terrain::Wall;
		}
	}

	auto& data = fix.add_object();
	auto& coll = fix.collision.query(data.id);
	sf::Vector2u pos{1, 1};
	core::SpawnHelper helper{fix.collision, fix.movement, dungeon, data.id};	
	BOOST_CHECK(!core::getFreePosition(helper, pos, 5));
}

BOOST_AUTO_TEST_CASE(finding_free_pos_can_find_rare_spots) {
	auto& fix = Singleton<TeleportFixture>::get();
	fix.reset();

	// modify terrain
	auto& dungeon = fix.dungeon[1u];
	for (auto y = 0u; y < 10u; ++y) {
		for (auto x = 0u; x < 12u; ++x) {
			dungeon.getCell({x, y}).terrain = core::Terrain::Wall;
		}
	}

	dungeon.getCell({1, 1}).terrain = core::Terrain::Floor;
	dungeon.getCell({1, 3}).terrain = core::Terrain::Floor;

	auto& data = fix.add_object();
	auto& coll = fix.collision.query(data.id);
	sf::Vector2u pos{3, 3};
	core::spawn(dungeon, data, pos);

	core::SpawnHelper helper{fix.collision, fix.movement, dungeon, data.id};	
	BOOST_CHECK(!core::getFreePosition(helper, pos, 1));
	BOOST_CHECK(core::getFreePosition(helper, pos, 2));
	BOOST_CHECK_VECTOR_EQUAL(pos, sf::Vector2u(1u, 1u));
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(teleport_trigger_doesnt_expire) {
	auto& fix = Singleton<TeleportFixture>::get();
	fix.reset();

	auto& data = fix.add_object();
	core::MoveSender move;
	core::TeleportSender teleport_sender;
	core::TeleportTrigger trigger{move, teleport_sender, fix.movement,
		fix.collision, fix.dungeon, 2u, {5u, 7u}};
	core::spawn(fix.dungeon[1u], data, {1u, 1u});

	trigger.execute(data.id);
	BOOST_CHECK(!trigger.isExpired());
}

BOOST_AUTO_TEST_CASE(teleport_trigger_moves_to_given_spot) {
	auto& fix = Singleton<TeleportFixture>::get();
	fix.reset();

	auto& data = fix.add_object();
	core::MoveSender move;
	core::TeleportSender teleport_sender;
	core::TeleportTrigger trigger{move, teleport_sender, fix.movement,
		fix.collision, fix.dungeon, 2u, {5u, 7u}};
	core::spawn(fix.dungeon[1u], data, {1u, 1u});

	trigger.execute(data.id);

	BOOST_CHECK_VECTOR_CLOSE(data.pos, sf::Vector2f(5.f, 7.f), 0.0001f);
	BOOST_CHECK_VECTOR_EQUAL(data.target, sf::Vector2u(5u, 7u));
	BOOST_CHECK_EQUAL(data.scene, 2u);
	BOOST_CHECK(data.has_changed);
}

BOOST_AUTO_TEST_CASE(teleport_trigger_causes_teleport_event) {
	auto& fix = Singleton<TeleportFixture>::get();
	fix.reset();

	auto& data = fix.add_object();
	core::MoveSender move;
	core::TeleportSender teleport_sender;
	core::TeleportTrigger trigger{move, teleport_sender, fix.movement,
		fix.collision, fix.dungeon, 2u, {5u, 7u}};
	core::spawn(fix.dungeon[1u], data, {1u, 1u});

	trigger.execute(data.id);

	auto const & events = teleport_sender.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, data.id);
	BOOST_CHECK_EQUAL(events[0].src_scene, 1u);
	BOOST_CHECK_VECTOR_EQUAL(events[0].src_pos, sf::Vector2u(1u, 1u));
	BOOST_CHECK_EQUAL(events[0].dst_scene, 2u);
	BOOST_CHECK_VECTOR_EQUAL(events[0].dst_pos, sf::Vector2u(5u, 7u));
}

BOOST_AUTO_TEST_CASE(teleport_trigger_stops_object_after_teleport) {
	auto& fix = Singleton<TeleportFixture>::get();
	fix.reset();

	auto& data = fix.add_object();
	core::MoveSender move;
	core::TeleportSender teleport_sender;
	core::TeleportTrigger trigger{move, teleport_sender, fix.movement,
		fix.collision, fix.dungeon, 2u, {5u, 7u}};
	core::spawn(fix.dungeon[1u], data, {1u, 1u});

	trigger.execute(data.id);

	BOOST_CHECK_VECTOR_EQUAL(data.move, sf::Vector2i());
	BOOST_CHECK_VECTOR_EQUAL(data.next_move, sf::Vector2i());
}

BOOST_AUTO_TEST_CASE(teleport_trigger_propagates_tile_reached) {
	auto& fix = Singleton<TeleportFixture>::get();
	fix.reset();

	auto& data = fix.add_object();
	core::MoveSender move;
	core::TeleportSender teleport_sender;
	core::TeleportTrigger trigger{move, teleport_sender, fix.movement,
		fix.collision, fix.dungeon, 2u, {5u, 7u}};
	core::spawn(fix.dungeon[1u], data, {1u, 1u});

	trigger.execute(data.id);

	auto const events = move.data();
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK_EQUAL(events[0].actor, data.id);
	BOOST_CHECK_VECTOR_EQUAL(events[0].target, sf::Vector2u(5u, 7u));
	BOOST_CHECK(events[0].type == core::MoveEvent::Left);
}

BOOST_AUTO_TEST_CASE(teleport_trigger_fails_if_position_unaccessable) {
	auto& fix = Singleton<TeleportFixture>::get();
	fix.reset();

	auto& data = fix.add_object();
	core::MoveSender move;
	core::TeleportSender teleport_sender;
	core::TeleportTrigger trigger{move, teleport_sender, fix.movement,
		fix.collision, fix.dungeon, 2u, {8u, 9u}};
	core::spawn(fix.dungeon[1u], data, {1u, 1u});
	data.has_changed = false;

	// modify terrain
	auto& dungeon = fix.dungeon[2u];
	for (auto y = 0u; y < 10u; ++y) {
		for (auto x = 0u; x < 12u; ++x) {
			dungeon.getCell({x, y}).terrain = core::Terrain::Wall;
		}
	}

	trigger.execute(data.id);

	BOOST_CHECK(move.data().empty());
	BOOST_CHECK_VECTOR_CLOSE(data.pos, sf::Vector2f(1.f, 1.f), 0.0001f);
	BOOST_CHECK_VECTOR_EQUAL(data.target, sf::Vector2u(1u, 1u));
	BOOST_CHECK_EQUAL(data.scene, 1u);
	BOOST_CHECK(!data.has_changed);
}

BOOST_AUTO_TEST_SUITE_END()

#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>
#include <testsuite/singleton.hpp>

#include <utils/algorithm.hpp>
#include <game/builder.hpp>
#include <game/navigator.hpp>
#include <game/path.hpp>

struct PathFixture {
	sf::Texture dummy;
	core::LogContext log;
	core::CollisionManager collision;
	core::Dungeon dungeon;
	std::unique_ptr<game::Navigator> navi;
	game::RoomTemplate room;

	core::IdManager ids;
	std::vector<core::ObjectID> objects;

	PathFixture()
		: dummy{}
		, log{}
		, collision{}
		, dungeon{1u, dummy, {20u, 15u}, {16.f, 16.f}}
		, navi{nullptr}
		, ids{}
		, objects{} {

		game::DungeonGraph graph{{4u, 3u}};
		game::NavigationScene scene{collision, dungeon};
		//	X	X---X	X
		//	|	|	 	|
		//	X---X---X---X
		//	|	 	|
		//	X	X---X   X
		game::DungeonBuilder builder{{20u, 15u}};
		// create rooms
		for (auto y = 0u; y < 3u; ++y) {
			for (auto x = 0u; x < 4u; ++x) {
				graph.addNode({x, y});
				builder.rooms.emplace_back(5u * x, 5u * y, room);
			}
		}
		// create corridors
		builder.paths.emplace_back(1u, 0u, 2u, 0u);
		builder.paths.emplace_back(0u, 0u, 0u, 1u);
		builder.paths.emplace_back(1u, 0u, 1u, 1u);
		builder.paths.emplace_back(3u, 0u, 3u, 1u);
		builder.paths.emplace_back(0u, 1u, 1u, 1u);
		builder.paths.emplace_back(1u, 1u, 2u, 1u);
		builder.paths.emplace_back(2u, 1u, 3u, 1u);
		builder.paths.emplace_back(0u, 1u, 0u, 2u);
		builder.paths.emplace_back(2u, 1u, 2u, 2u);
		builder.paths.emplace_back(1u, 2u, 2u, 2u);
		for (auto& path : builder.paths) {
			graph.addPath(path.origin, path.target);
			path.origin.x = path.origin.x * 5 + 2;
			path.origin.y = path.origin.y * 5 + 2;
			path.target.x = path.target.x * 5 + 2;
			path.target.y = path.target.y * 5 + 2;
		}
		// build dungeon
		rpg::TilesetTemplate tileset;
		tileset.tilesize = {16u, 16u};
		tileset.floors.emplace_back(0u, 0u);
		tileset.walls.emplace_back(16u, 0u);
		tileset.tileset = &dummy;
		game::BuildSettings settings;
		settings.path_width = 2u;
		builder(tileset, dungeon, settings);

		navi = std::make_unique<game::Navigator>(
			std::move(graph), std::move(scene));
	}

	core::ObjectID addActor(sf::Vector2u const& pos) {
		auto id = ids.acquire();
		objects.push_back(id);
		collision.acquire(id);
		dungeon.getCell(pos).entities.push_back(id);
		return id;
	}

	void reset() {
		// clear dungeon
		for (auto y = 0u; y < 15u; ++y) {
			for (auto x = 0u; x < 20u; ++x) {
				dungeon.getCell({x, y}).entities.clear();
			}
		}
		// remove components
		for (auto id : objects) {
			collision.release(id);
		}
		objects.clear();
		ids.reset();
		collision.cleanup();
	}
};

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(path_test)

BOOST_AUTO_TEST_CASE(incomplete_pathfind_results_in_not_ready_future) {
	auto& fix = Singleton<PathFixture>::get();
	fix.reset();

	game::PathSystem system{fix.log};
	system.addScene(1u, *fix.navi);
	auto id = fix.addActor({2u, 3u});
	auto future =
		system.schedule(id, 1u, {0u, 0u}, {3, 0});
	// expect: (0,0) --> (0,1) --> (1,1) --> (2,1) --> (3,1) --> (3,0)
	auto status = future.wait_for(std::chrono::milliseconds(0));
	BOOST_CHECK(status != std::future_status::ready);
}

/*
BOOST_AUTO_TEST_CASE(can_perform_broadphase_searching) {
	auto& fix = Singleton<PathFixture>::get();
	fix.reset();

	game::PathSystem system{fix.log};
	system.addScene(1u, *fix.navi);
	auto id = fix.addActor({2u, 3u});
	auto future =
		system.schedule(id, 1u, {0u, 0u}, {3, 0});
	// expect: (0,0) --> (0,1) --> (1,1) --> (2,1) --> (3,1) --> (3,0)
	auto n = system.calculate(sf::milliseconds(1000u));
	BOOST_REQUIRE(future.valid());
	auto path = future.get();
	BOOST_CHECK_EQUAL(n, 1u);
	BOOST_REQUIRE_EQUAL(path.size(), 6u);
	BOOST_CHECK_VECTOR_EQUAL(path.at(5), sf::Vector2u(0u, 0u));
	BOOST_CHECK_VECTOR_EQUAL(path.at(4), sf::Vector2u(0u, 1u));
	BOOST_CHECK_VECTOR_EQUAL(path.at(3), sf::Vector2u(1u, 1u));
	BOOST_CHECK_VECTOR_EQUAL(path.at(2), sf::Vector2u(2u, 1u));
	BOOST_CHECK_VECTOR_EQUAL(path.at(1), sf::Vector2u(3u, 1u));
	BOOST_CHECK_VECTOR_EQUAL(path.at(0), sf::Vector2u(3u, 0u));
}
*/

BOOST_AUTO_TEST_CASE(can_perform_narrowphase_searching) {
	auto& fix = Singleton<PathFixture>::get();
	fix.reset();

	game::PathSystem system{fix.log};
	system.addScene(1u, *fix.navi);
	auto id = fix.addActor({2u, 3u});
	auto future =
		system.schedule(id, 1u, {1u, 3u}, {2, 7});
	// expect: (1,3) --> (1,4) --> (1,5) --> (2,6) --> (2,7)
	auto n = system.calculate(sf::milliseconds(1000u));
	BOOST_REQUIRE(future.valid());
	auto path = future.get();
	BOOST_CHECK_EQUAL(n, 1u);
	BOOST_REQUIRE_EQUAL(path.size(), 5u);
	BOOST_CHECK_VECTOR_EQUAL(path.at(4), sf::Vector2u(1u, 3u));
	BOOST_CHECK_VECTOR_EQUAL(path.at(3), sf::Vector2u(2u, 4u));
	BOOST_CHECK_VECTOR_EQUAL(path.at(2), sf::Vector2u(2u, 5u));
	BOOST_CHECK_VECTOR_EQUAL(path.at(1), sf::Vector2u(2u, 6u));
	BOOST_CHECK_VECTOR_EQUAL(path.at(0), sf::Vector2u(2u, 7u));
}

/*
BOOST_AUTO_TEST_CASE(impossible_path_contains_only_source_position) {
	auto& fix = Singleton<PathFixture>::get();
	fix.reset();

	game::PathSystem system{fix.log};
	system.addScene(1u, *fix.navi);
	auto id = fix.addActor({2u, 3u});
	auto future =
		system.schedule(game::PathPhase::Broad, id, 1u, {0u, 1u}, {3, 2});
	auto n = system.calculate(sf::milliseconds(1000u));
	BOOST_REQUIRE(future.valid());
	auto path = future.get();
	BOOST_CHECK_EQUAL(n, 1u);
	BOOST_REQUIRE_EQUAL(path.size(), 1u);
	BOOST_CHECK_VECTOR_EQUAL(path.at(0), sf::Vector2u(0u, 1u));
}
*/

/*
BOOST_AUTO_TEST_CASE(can_calculate_two_paths) {
	auto& fix = Singleton<PathFixture>::get();
	fix.reset();

	game::PathSystem system{fix.log};
	system.addScene(1u, *fix.navi);
	auto id = fix.addActor({2u, 3u});
	auto future =
		system.schedule(game::PathPhase::Broad, id, 1u, {0u, 0u}, {3, 0});
	auto future2 =
		system.schedule(game::PathPhase::Broad, id, 1u, {2u, 0u}, {0u, 2u});
	auto n = system.calculate(sf::milliseconds(1000u));
	BOOST_REQUIRE(future.valid());
	BOOST_REQUIRE(future2.valid());
	auto path = future.get();
	auto other = future2.get();
	BOOST_CHECK_EQUAL(n, 2u);
	BOOST_REQUIRE_EQUAL(path.size(), 6u);
	BOOST_CHECK_VECTOR_EQUAL(path.at(5), sf::Vector2u(0u, 0u));
	BOOST_CHECK_VECTOR_EQUAL(path.at(4), sf::Vector2u(0u, 1u));
	BOOST_CHECK_VECTOR_EQUAL(path.at(3), sf::Vector2u(1u, 1u));
	BOOST_CHECK_VECTOR_EQUAL(path.at(2), sf::Vector2u(2u, 1u));
	BOOST_CHECK_VECTOR_EQUAL(path.at(1), sf::Vector2u(3u, 1u));
	BOOST_CHECK_VECTOR_EQUAL(path.at(0), sf::Vector2u(3u, 0u));
	BOOST_REQUIRE_EQUAL(other.size(), 5u);
	BOOST_CHECK_VECTOR_EQUAL(other.at(4), sf::Vector2u(2u, 0u));
	BOOST_CHECK_VECTOR_EQUAL(other.at(3), sf::Vector2u(1u, 0u));
	BOOST_CHECK_VECTOR_EQUAL(other.at(2), sf::Vector2u(1u, 1u));
	BOOST_CHECK_VECTOR_EQUAL(other.at(1), sf::Vector2u(0u, 1u));
	BOOST_CHECK_VECTOR_EQUAL(other.at(0), sf::Vector2u(0u, 2u));
}
*/

BOOST_AUTO_TEST_SUITE_END()

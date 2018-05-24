#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>
#include <testsuite/singleton.hpp>

#include <utils/algorithm.hpp>
#include <core/event.hpp>
#include <core/teleport.hpp>
#include <game/navigator.hpp>

BOOST_AUTO_TEST_SUITE(navigator_test)

BOOST_AUTO_TEST_CASE(isolated_node_has_no_neighbors) {
	game::DungeonGraph grid{{3u, 3u}};
	grid.addNode({2u, 2u});

	auto neighbors = grid.getNeighbors(1u, {2u, 2u});
	BOOST_CHECK(neighbors.empty());
}

BOOST_AUTO_TEST_CASE(node_can_have_one_path) {
	game::DungeonGraph grid{{3u, 3u}};
	grid.addNode({1u, 2u});
	grid.addNode({2u, 2u});
	grid.addPath({1u, 2u}, {2u, 2u});

	auto neighbors = grid.getNeighbors(1u, {2u, 2u});
	BOOST_REQUIRE_EQUAL(neighbors.size(), 1u);
	BOOST_CHECK_VECTOR_EQUAL(neighbors[0], sf::Vector2u(1u, 2u));
}

BOOST_AUTO_TEST_CASE(node_can_have_multiple_path) {
	game::DungeonGraph grid{{3u, 3u}};
	grid.addNode({0u, 1u});
	grid.addNode({1u, 1u});
	grid.addNode({1u, 2u});
	grid.addPath({0u, 1u}, {1u, 1u});
	grid.addPath({1u, 1u}, {1u, 2u});

	auto neighbors = grid.getNeighbors(1u, {1u, 1u});
	BOOST_REQUIRE_EQUAL(neighbors.size(), 2u);
	BOOST_CHECK_VECTOR_EQUAL(neighbors[0], sf::Vector2u(0u, 1u));
	BOOST_CHECK_VECTOR_EQUAL(neighbors[1], sf::Vector2u(1u, 2u));
}

// ---------------------------------------------------------------------------

/// @note broadphase not implemented yet, hence no testing
/*
BOOST_AUTO_TEST_CASE(can_navigate_at_broadphase) {
	sf::Texture dummy;
	game::DungeonGraph grid{{3u, 3u}};
	grid.addNode({0u, 1u});
	grid.addNode({1u, 1u});
	grid.addNode({1u, 2u});
	grid.addPath({0u, 1u}, {1u, 1u});
	grid.addPath({1u, 1u}, {1u, 2u});
	core::MovementManager movement;
	core::CollisionManager collision;
	core::Dungeon dungeon{1u, dummy, {30u, 30u}, {8.f, 8.f}};
	game::NavigationScene scene{movement, collision, dungeon};
	game::Navigator navigator{std::move(grid), std::move(scene)};

	auto& astar = navigator.broadphase;
	auto path = astar(1u, {1u, 2u}, {0u, 1u}, 5u);
	BOOST_REQUIRE_EQUAL(path.size(), 3u);
	BOOST_CHECK_VECTOR_EQUAL(path[0], sf::Vector2u(0u, 1u));
	BOOST_CHECK_VECTOR_EQUAL(path[1], sf::Vector2u(1u, 1u));
	BOOST_CHECK_VECTOR_EQUAL(path[2], sf::Vector2u(1u, 2u));
}
*/

BOOST_AUTO_TEST_CASE(can_navigate_at_narrowphase) {
	sf::Texture dummy;
	game::DungeonGraph grid{{}};
	core::MovementManager movement;
	core::CollisionManager collision;
	core::Dungeon dungeon{1u, dummy, {30u, 30u}, {8.f, 8.f}};
	game::NavigationScene scene{movement, collision, dungeon};
	game::Navigator navigator{std::move(grid), std::move(scene)};

	core::ObjectID actor{17u};
	movement.acquire(actor);
	collision.acquire(actor);
	//	.
	//	... .
	//	.   .
	//	.....
	//	.
	dungeon.getCell({2u, 2u}).terrain = core::Terrain::Floor;
	dungeon.getCell({2u, 3u}).terrain = core::Terrain::Floor;
	dungeon.getCell({2u, 4u}).terrain = core::Terrain::Wall;
	dungeon.getCell({2u, 5u}).terrain = core::Terrain::Floor;
	dungeon.getCell({2u, 6u}).terrain = core::Terrain::Floor;
	dungeon.getCell({3u, 3u}).terrain = core::Terrain::Floor;
	dungeon.getCell({4u, 3u}).terrain = core::Terrain::Floor;
	dungeon.getCell({5u, 3u}).terrain = core::Terrain::Floor;
	dungeon.getCell({6u, 3u}).terrain = core::Terrain::Floor;
	dungeon.getCell({6u, 4u}).terrain = core::Terrain::Floor;
	dungeon.getCell({6u, 5u}).terrain = core::Terrain::Floor;
	dungeon.getCell({5u, 5u}).terrain = core::Terrain::Floor;
	dungeon.getCell({4u, 5u}).terrain = core::Terrain::Floor;
	dungeon.getCell({3u, 5u}).terrain = core::Terrain::Floor;

	auto& astar = navigator.narrowphase;
	auto path = astar(actor, {2u, 3u}, {2u, 5u}, 20u);
	// expect right path without (impossible) shortcut tile
	BOOST_REQUIRE_EQUAL(path.size(), 9u);
	BOOST_CHECK(!utils::contains(path, sf::Vector2u(2u, 4u)));
}

BOOST_AUTO_TEST_CASE(teleport_triggers_are_avoided_at_narrowphase) {
	sf::Texture dummy;
	game::DungeonGraph grid{{}};
	core::MovementManager movement;
	core::CollisionManager collision;
	core::Dungeon dungeon{1u, dummy, {30u, 30u}, {8.f, 8.f}};
	game::NavigationScene scene{movement, collision, dungeon};
	game::Navigator navigator{std::move(grid), std::move(scene)};
	
	core::MoveSender move_sender;
	core::TeleportSender teleport_sender;
	core::DungeonSystem dungeonsystem;
	
	core::ObjectID actor{17u};
	movement.acquire(actor);
	collision.acquire(actor);

	//	.
	//	...T.	[T]eleport
	//	.   .
	//	.....
	//	.
	dungeon.getCell({2u, 2u}).terrain = core::Terrain::Floor;
	dungeon.getCell({2u, 3u}).terrain = core::Terrain::Floor;
	{
		auto& cell = dungeon.getCell({2u, 4u});
		cell.terrain = core::Terrain::Floor;
		cell.trigger = std::make_unique<core::TeleportTrigger>(move_sender,
			teleport_sender, movement, collision, dungeonsystem, dungeon.id,
			sf::Vector2u{3u, 3u});
	}
	dungeon.getCell({2u, 5u}).terrain = core::Terrain::Floor;
	dungeon.getCell({2u, 6u}).terrain = core::Terrain::Floor;
	dungeon.getCell({3u, 3u}).terrain = core::Terrain::Floor;
	dungeon.getCell({4u, 3u}).terrain = core::Terrain::Floor;
	dungeon.getCell({5u, 3u}).terrain = core::Terrain::Floor;
	dungeon.getCell({6u, 3u}).terrain = core::Terrain::Floor;
	dungeon.getCell({6u, 4u}).terrain = core::Terrain::Floor;
	dungeon.getCell({6u, 5u}).terrain = core::Terrain::Floor;
	dungeon.getCell({5u, 5u}).terrain = core::Terrain::Floor;
	dungeon.getCell({4u, 5u}).terrain = core::Terrain::Floor;
	dungeon.getCell({3u, 5u}).terrain = core::Terrain::Floor;

	auto& astar = navigator.narrowphase;
	auto path = astar(actor, {2u, 3u}, {2u, 5u}, 20u);
	BOOST_REQUIRE_EQUAL(path.size(), 9u);
}

BOOST_AUTO_TEST_CASE(diagonal_movements_have_higher_priority_for_going_south_east) {
	sf::Texture dummy;
	game::DungeonGraph grid{{}};
	core::MovementManager movement;
	core::CollisionManager collision;
	core::Dungeon dungeon{1u, dummy, {30u, 30u}, {8.f, 8.f}};
	game::NavigationScene scene{movement, collision, dungeon};
	game::Navigator navigator{std::move(grid), std::move(scene)};
	
	core::ObjectID actor{17u};
	movement.acquire(actor);
	collision.acquire(actor);
	//	S.	[S]ource
	//	..
	//	..
	//	..
	//	.D	[D]estination
	sf::Vector2u pos;
	for (pos.y = 2u; pos.y <= 6u; ++pos.y) {
		for (pos.x = 2u; pos.x <= 3u; ++pos.x) {
			dungeon.getCell(pos).terrain = core::Terrain::Floor;
		}
	}

	auto& astar = navigator.narrowphase;
	auto path = astar(actor, {2u, 2u}, {3u, 6u}, 20u);
	BOOST_REQUIRE_EQUAL(path.size(), 5u);
	BOOST_CHECK_VECTOR_EQUAL(path[4], sf::Vector2u(2u, 2u));
	BOOST_CHECK_VECTOR_EQUAL(path[3], sf::Vector2u(3u, 3u));
	BOOST_CHECK_VECTOR_EQUAL(path[2], sf::Vector2u(3u, 4u));
}

BOOST_AUTO_TEST_CASE(diagonal_movements_have_higher_priority_for_going_south_west) {
	sf::Texture dummy;
	game::DungeonGraph grid{{}};
	core::MovementManager movement;
	core::CollisionManager collision;
	core::Dungeon dungeon{1u, dummy, {30u, 30u}, {8.f, 8.f}};
	game::NavigationScene scene{movement, collision, dungeon};
	game::Navigator navigator{std::move(grid), std::move(scene)};
	
	core::ObjectID actor{17u};
	movement.acquire(actor);
	collision.acquire(actor);
	//	.S	[S]ource
	//	..
	//	..
	//	..
	//	D.	[D]estination
	sf::Vector2u pos;
	for (pos.y = 2u; pos.y <= 6u; ++pos.y) {
		for (pos.x = 2u; pos.x <= 3u; ++pos.x) {
			dungeon.getCell(pos).terrain = core::Terrain::Floor;
		}
	}

	auto& astar = navigator.narrowphase;
	auto path = astar(actor, {3u, 2u}, {2u, 6u}, 20u);
	BOOST_REQUIRE_EQUAL(path.size(), 5u);
	BOOST_CHECK_VECTOR_EQUAL(path[4], sf::Vector2u(3u, 2u));
	BOOST_CHECK_VECTOR_EQUAL(path[3], sf::Vector2u(2u, 3u));
	BOOST_CHECK_VECTOR_EQUAL(path[2], sf::Vector2u(2u, 4u));
}

BOOST_AUTO_TEST_CASE(diagonal_movements_have_higher_priority_for_going_north_east) {
	sf::Texture dummy;
	game::DungeonGraph grid{{}};
	core::MovementManager movement;
	core::CollisionManager collision;
	core::Dungeon dungeon{1u, dummy, {30u, 30u}, {8.f, 8.f}};
	game::NavigationScene scene{movement, collision, dungeon};
	game::Navigator navigator{std::move(grid), std::move(scene)};
	
	core::ObjectID actor{17u};
	movement.acquire(actor);
	collision.acquire(actor);
	//	....D	[S]ource
	//	S....	[D]estination
	sf::Vector2u pos;
	for (pos.y = 2u; pos.y <= 3u; ++pos.y) {
		for (pos.x = 2u; pos.x <= 6u; ++pos.x) {
			dungeon.getCell(pos).terrain = core::Terrain::Floor;
		}
	}

	auto& astar = navigator.narrowphase;
	auto path = astar(actor, {2u, 3u}, {6u, 2u}, 20u);
	BOOST_REQUIRE_EQUAL(path.size(), 5u);
	BOOST_CHECK_VECTOR_EQUAL(path[4], sf::Vector2u(2u, 3u));
	BOOST_CHECK_VECTOR_EQUAL(path[3], sf::Vector2u(3u, 2u));
	BOOST_CHECK_VECTOR_EQUAL(path[2], sf::Vector2u(4u, 2u));
}

BOOST_AUTO_TEST_CASE(diagonal_movements_have_higher_priority_north_west) {
	sf::Texture dummy;
	game::DungeonGraph grid{{}};
	core::MovementManager movement;
	core::CollisionManager collision;
	core::Dungeon dungeon{1u, dummy, {30u, 30u}, {8.f, 8.f}};
	game::NavigationScene scene{movement, collision, dungeon};
	game::Navigator navigator{std::move(grid), std::move(scene)};
	
	core::ObjectID actor{17u};
	movement.acquire(actor);
	collision.acquire(actor);
	//	D....	[S]ource
	//	....S	[D]estination
	sf::Vector2u pos;
	for (pos.y = 2u; pos.y <= 3u; ++pos.y) {
		for (pos.x = 2u; pos.x <= 6u; ++pos.x) {
			dungeon.getCell(pos).terrain = core::Terrain::Floor;
		}
	}

	auto& astar = navigator.narrowphase;
	auto path = astar(actor, {6u, 3u}, {2u, 2u}, 20u);
	BOOST_REQUIRE_EQUAL(path.size(), 5u);
	BOOST_CHECK_VECTOR_EQUAL(path[4], sf::Vector2u(6u, 3u));
	BOOST_CHECK_VECTOR_EQUAL(path[3], sf::Vector2u(5u, 2u));
	BOOST_CHECK_VECTOR_EQUAL(path[2], sf::Vector2u(4u, 2u));
}

BOOST_AUTO_TEST_SUITE_END()

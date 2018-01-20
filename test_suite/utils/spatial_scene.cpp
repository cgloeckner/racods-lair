#include <vector>
#include <boost/test/unit_test.hpp>

#include <utils/spatial_scene.hpp>

using EntityID = std::size_t;

struct TestCell {
};

using TestScene =
	utils::SpatialScene<TestCell, EntityID, utils::GridMode::Orthogonal>;

using TestAABBQuery = utils::AABBEntityQuery<EntityID>;
using TestCircQuery = utils::CircEntityQuery<EntityID>;

// ----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(spatial_scene_test)

BOOST_AUTO_TEST_CASE(aabbquery_returns_specified_range) {
	TestAABBQuery query{{3.5f, 5.75f}, {2.f, 3.f}};
	auto range = query.getRange();
	
	BOOST_CHECK_EQUAL(range.left,   2u);
	BOOST_CHECK_EQUAL(range.top,    4u);
	BOOST_CHECK_EQUAL(range.width,  2u);
	BOOST_CHECK_EQUAL(range.height, 3u);
}

BOOST_AUTO_TEST_CASE(aabbquery_returns_minimal_range) {
	TestAABBQuery query{{3.5f, 4.5f}, {0.f, 0.f}};
	auto range = query.getRange();
	
	BOOST_CHECK_EQUAL(range.left,   3u);
	BOOST_CHECK_EQUAL(range.top,    4u);
	BOOST_CHECK_EQUAL(range.width,  0u);
	BOOST_CHECK_EQUAL(range.height, 0u);
}

BOOST_AUTO_TEST_CASE(aabbquery_collects_from_cell) {
	std::vector<EntityID> entities;
	entities.push_back(3u);
	entities.push_back(7u);
	
	TestAABBQuery query{{3.f, 4.f}, {2.f, 3.f}};
	query({}, entities);
	
	BOOST_REQUIRE_EQUAL(query.entities.size(), 2u);
	BOOST_CHECK(utils::contains(query.entities, 3u));
	BOOST_CHECK(utils::contains(query.entities, 7u));
}

// ----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(circquery_returns_specific_range) {
	TestCircQuery query{{4.5f, 5.5f}, 3.3f};
	auto range = query.getRange();
	
	BOOST_CHECK_EQUAL(range.left,   1u);
	BOOST_CHECK_EQUAL(range.top,    2u);
	BOOST_CHECK_EQUAL(range.width,  7u);
	BOOST_CHECK_EQUAL(range.height, 7u);
}

BOOST_AUTO_TEST_CASE(circquery_collects_from_cell) {
	std::vector<EntityID> entities;
	entities.push_back(3u);
	entities.push_back(7u);
	
	TestCircQuery query{{3.f, 4.f}, 2.f};
	query({4.f, 4.f}, entities);
	
	BOOST_REQUIRE_EQUAL(query.entities.size(), 2u);
	BOOST_CHECK(utils::contains(query.entities, 3u));
	BOOST_CHECK(utils::contains(query.entities, 7u));
}

BOOST_AUTO_TEST_CASE(circquery_does_not_collect_from_cell_that_is_too_far_away) {
	std::vector<EntityID> entities;
	entities.push_back(3u);
	entities.push_back(7u);
	
	TestCircQuery query{{3.f, 3.f}, 2.f};
	query({1.f, 1.f}, entities);
	
	BOOST_REQUIRE_EQUAL(query.entities.size(), 0u);
}

// ----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(scene_ctor_allocates_nodes) {
	sf::Texture tileset;
	TestScene scene{1u, tileset, {10u, 8u}, {32.f, 32.f}};
	BOOST_CHECK_NO_THROW(scene.getCell({5u, 7u}));
}

BOOST_AUTO_TEST_CASE(scene_has_valid_pos) {
	sf::Texture tileset;
	TestScene scene{1u, tileset, {10u, 8u}, {32.f, 32.f}};
	BOOST_CHECK(scene.has({5u, 7u}));
}

BOOST_AUTO_TEST_CASE(scene_invalid_pos_are_outside_bounds) {
	sf::Texture tileset;
	TestScene scene{1u, tileset, {10u, 8u}, {32.f, 32.f}};
	auto v = static_cast<unsigned int>(-1);
	BOOST_CHECK(!scene.has({v, 0u}));
	BOOST_CHECK(!scene.has({0u, v}));
	BOOST_CHECK(!scene.has({10u, 0u}));
	BOOST_CHECK(!scene.has({0u, 8u}));
	BOOST_CHECK(!scene.has({11u, 7u}));
	BOOST_CHECK(!scene.has({9u, 10u}));
}

BOOST_AUTO_TEST_CASE(scene_nothrow_when_query_valid_cell) {
	sf::Texture tileset;
	TestScene scene{1u, tileset, {10u, 8u}, {32.f, 32.f}};
	BOOST_CHECK_NO_THROW(scene.getCell({7u, 5u}));
	auto const& const_scene = scene;
	BOOST_CHECK_NO_THROW(const_scene.getCell({7u, 5u}));
}

BOOST_AUTO_TEST_CASE(scene_throws_when_query_invalid_cell) {
	sf::Texture tileset;
	TestScene scene{1u, tileset, {10u, 8u}, {32.f, 32.f}};
	BOOST_CHECK_THROW(scene.getCell({7u, 12u}), std::out_of_range);
	BOOST_CHECK_THROW(scene.getCell({13u, 6u}), std::out_of_range);
	auto const& const_scene = scene;
	BOOST_CHECK_THROW(const_scene.getCell({7u, 12u}), std::out_of_range);
}

// -------------------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(scene_traverse_queries_all_requested_entities) {
	sf::Texture tileset;
	TestScene scene{1u, tileset, {10u, 8u}, {32.f, 32.f}};
	
	scene.getCell({5u, 4u}).entities.push_back(3);
	scene.getCell({4u, 5u}).entities.push_back(10);
	
	TestAABBQuery query{{4.f, 5.f}, {3.f, 4.f}};
	scene.traverse(query);
	
	BOOST_REQUIRE_EQUAL(query.entities.size(), 2u);
	BOOST_CHECK(utils::contains(query.entities, 3u));
	BOOST_CHECK(utils::contains(query.entities, 10u));
}

BOOST_AUTO_TEST_CASE(scene_traverse_ignores_outside_entities) {
	sf::Texture tileset;
	TestScene scene{1u, tileset, {10u, 8u}, {32.f, 32.f}};
	
	scene.getCell({1u, 2u}).entities.push_back(3);
	scene.getCell({4u, 5u}).entities.push_back(10);
	
	TestAABBQuery query{{4.f, 5.f}, {3.f, 4.f}};
	scene.traverse(query);
	
	BOOST_REQUIRE_EQUAL(query.entities.size(), 1u);
	BOOST_CHECK(!utils::contains(query.entities, 3u));
	BOOST_CHECK(utils::contains(query.entities, 10u));
}

BOOST_AUTO_TEST_CASE(scene_traverse_minimal_aabb_ignores_neighbors) {
	sf::Texture tileset;
	TestScene scene{1u, tileset, {10u, 8u}, {32.f, 32.f}};
	
	int i{10};
	for (auto y = 3u; y <= 5u; ++y) {
		for (auto x = 4u; x <= 6u; ++x) {
			scene.getCell({x, y}).entities.push_back(i++);
		}
	}
	// (5,4) contains #14
	
	TestAABBQuery query{{5.f, 4.f}, {0.f, 0.f}};
	scene.traverse(query);
	
	BOOST_REQUIRE_EQUAL(query.entities.size(), 1u);
	BOOST_CHECK(utils::contains(query.entities, 14u));
}

BOOST_AUTO_TEST_CASE(scene_traverse_minimal_circ_ignores_neighbors) {
	sf::Texture tileset;
	TestScene scene{1u, tileset, {10u, 8u}, {32.f, 32.f}};
	
	int i{10};
	for (auto y = 3u; y <= 5u; ++y) {
		for (auto x = 4u; x <= 6u; ++x) {
			scene.getCell({x, y}).entities.push_back(i++);
		}
	}
	// (5,4) contains #14
	
	TestAABBQuery query{{5.f, 4.f}, {0.f, 0.f}};
	scene.traverse(query);
	
	BOOST_REQUIRE_EQUAL(query.entities.size(), 1u);
	BOOST_CHECK(utils::contains(query.entities, 14u));
}

BOOST_AUTO_TEST_SUITE_END()

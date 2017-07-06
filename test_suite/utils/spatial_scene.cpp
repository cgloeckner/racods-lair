#include <vector>
#include <boost/test/unit_test.hpp>

#include <utils/spatial_scene.hpp>

using EntityID = std::size_t;

struct TestCell {};

using TestScene =
	utils::SpatialScene<TestCell, EntityID, utils::GridMode::Orthogonal>;

// ----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(spatial_scene_test)

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

BOOST_AUTO_TEST_SUITE_END()

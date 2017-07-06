#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>

#include <utils/assert.hpp>
#include <utils/lighting_system.hpp>

// ----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(LightingSystem_test)

BOOST_AUTO_TEST_CASE(LightingSystem_getFarPoint_returns_point_at_top) {
	sf::Vector2f origin{24.5, 53.f};
	sf::Vector2f pos{23.5, 35.f};
	sf::FloatRect box{10.f, 30.f, 40.f, 35.f};
	auto result = utils::getFarPoint(origin, pos, box);

	BOOST_CHECK_LE(result.y, 30.f);
}

BOOST_AUTO_TEST_CASE(LightingSystem_getFarPoint_returns_point_at_right) {
	sf::Vector2f origin{33.f, 50.f};
	sf::Vector2f pos{34.5, 53.f};
	sf::FloatRect box{10.f, 30.f, 40.f, 35.f};
	auto result = utils::getFarPoint(origin, pos, box);

	BOOST_CHECK_GE(result.x, 50.f);
}

BOOST_AUTO_TEST_CASE(LightingSystem_getFarPoint_returns_point_at_bottom) {
	sf::Vector2f origin{33.f, 50.f};
	sf::Vector2f pos{32., 60.f};
	sf::FloatRect box{10.f, 30.f, 40.f, 35.f};
	auto result = utils::getFarPoint(origin, pos, box);

	BOOST_CHECK_GE(result.y, 65.f);
}

BOOST_AUTO_TEST_CASE(LightingSystem_getFarPoint_returns_point_at_left) {
	sf::Vector2f origin{33.f, 50.f};
	sf::Vector2f pos{14.5f, 57.f};
	sf::FloatRect box{10.f, 30.f, 40.f, 35.f};
	auto result = utils::getFarPoint(origin, pos, box);

	BOOST_CHECK_LE(result.x, 10.f);
}

BOOST_AUTO_TEST_CASE(LightingSystem_getFarPoint_fails_if_origin_equals_pos) {
	sf::Vector2f pos{33.f, 50.f};
	sf::FloatRect box{10.f, 30.f, 40.f, 35.f};
	BOOST_CHECK_ASSERT(utils::getFarPoint(pos, pos, box));
}

BOOST_AUTO_TEST_CASE(
	LightingSystem_getFarPoint_works_correctly_if_pos_outside_box) {
	sf::Vector2f origin{33.f, 50.f};
	sf::Vector2f pos{70.f, 50.f};
	sf::FloatRect box{10.f, 30.f, 40.f, 35.f};
	auto result = utils::getFarPoint(origin, pos, box);

	BOOST_CHECK_GE(result.x, 50.f);
}

BOOST_AUTO_TEST_SUITE_END()

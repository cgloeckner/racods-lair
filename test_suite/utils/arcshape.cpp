#include <vector>
#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp> // BOOST_CHECK_VECTOR_CLOSE etc.

#include <utils/arcshape.hpp>

std::vector<sf::Vector2f> getPointsFromShape(utils::ArcShape const & s) {
	std::vector<sf::Vector2f> v;
	v.reserve(s.getPointCount());
	for (auto i = 0u; i < s.getPointCount(); ++i) {
		v.push_back(s.getPoint(i));
	}
	return v;
}

// -------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(arcshape_test)

BOOST_AUTO_TEST_CASE(arcshape_default_ctor) {
	utils::ArcShape s;
	
	BOOST_CHECK_CLOSE(s.getRadius(), 0.f, 0.0001f);
	BOOST_CHECK_CLOSE(s.getAngle(), 360.f, 0.00001f);
	BOOST_CHECK_VECTOR_CLOSE(s.getDirection(), sf::Vector2f(0.f, 1.f), 0.00001f);
	BOOST_CHECK_EQUAL(s.getPointCount(), 30u);
}

BOOST_AUTO_TEST_CASE(arcshape_ctor_sets_radius) {
	utils::ArcShape s{12.3f};
	
	BOOST_CHECK_CLOSE(s.getRadius(), 12.3f, 0.0001f);
}

BOOST_AUTO_TEST_CASE(arcshape_ctor_sets_radius_and_pointcount) {
	utils::ArcShape s{12.3f, 15u};
	
	BOOST_CHECK_CLOSE(s.getRadius(), 12.3f, 0.0001f);
	BOOST_CHECK_EQUAL(s.getPointCount(), 15u);
}

BOOST_AUTO_TEST_CASE(arcshape_can_change_angle) {
	utils::ArcShape s;
	s.setAngle(120.f);
	
	BOOST_CHECK_CLOSE(s.getAngle(), 120.f, 0.0001f);
}

BOOST_AUTO_TEST_CASE(arcshape_can_change_direction) {
	// note: this is not used at racod's lair because the shape
	// is always drawn using a transformation matrix
	// so this case is for the sake of being complete
	utils::ArcShape s;
	s.setDirection({-0.3f, 1.2f});
	
	BOOST_CHECK_VECTOR_CLOSE(s.getDirection(), sf::Vector2f(-0.3f, 1.2f), 0.0001f);
}

BOOST_AUTO_TEST_CASE(arcshape_returns_points_on_full_arc) {
	utils::ArcShape s{1.f, 4u};
	auto v = getPointsFromShape(s);
	
	BOOST_REQUIRE_EQUAL(v.size(), 4u);
	BOOST_CHECK_VECTOR_CLOSE(v[0u], sf::Vector2f(1.f, 0.f), 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(v[1u], sf::Vector2f(2.f, 1.f), 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(v[2u], sf::Vector2f(1.f, 2.f), 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(v[3u], sf::Vector2f(0.f, 1.f), 0.0001f);
}

BOOST_AUTO_TEST_CASE(arcshape_returns_points_on_half_arc) {
	utils::ArcShape s{1.f, 4u};
	s.setAngle(180.f);
	auto v = getPointsFromShape(s);
	
	BOOST_REQUIRE_EQUAL(v.size(), 4u);
	BOOST_CHECK_VECTOR_CLOSE(v[0u], sf::Vector2f(2.f, 1.f), 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(v[1u], sf::Vector2f(1.f, 2.f), 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(v[2u], sf::Vector2f(0.f, 1.f), 0.0001f);
	BOOST_CHECK_VECTOR_CLOSE(v[3u], sf::Vector2f(1.f, 1.f), 0.0001f); // center which means to skip rest of the circle
}

BOOST_AUTO_TEST_CASE(arcshape_returns_points_on_90_degree) {
	utils::ArcShape s{1.f, 4u};
	s.setAngle(90.f);
	auto v = getPointsFromShape(s);
	
	BOOST_REQUIRE_EQUAL(v.size(), 4u);
	BOOST_CHECK_VECTOR_CLOSE(v[0u], sf::Vector2f(1.707f, 1.707f), 0.001f);
	BOOST_CHECK_VECTOR_CLOSE(v[1u], sf::Vector2f(0.293f, 1.707f), 0.001f);
	BOOST_CHECK_VECTOR_CLOSE(v[2u], sf::Vector2f(1.f, 1.f), 0.001f); // center which means to skip rest of the circle
	BOOST_CHECK_VECTOR_CLOSE(v[3u], sf::Vector2f(1.f, 1.f), 0.001f); // center which means to skip rest of the circle
}

BOOST_AUTO_TEST_CASE(arcshape_returns_points_on_60_degree) {
	utils::ArcShape s{1.f, 10u};
	s.setAngle(120.f);
	auto v = getPointsFromShape(s);
	
	BOOST_REQUIRE_EQUAL(v.size(), 10u);
	BOOST_CHECK_VECTOR_CLOSE(v[0u], sf::Vector2f(1.866f, 1.5f  ), 0.001f);
	BOOST_CHECK_VECTOR_CLOSE(v[1u], sf::Vector2f(1.407f, 1.914f), 0.001f);
	BOOST_CHECK_VECTOR_CLOSE(v[2u], sf::Vector2f(0.792f, 1.978f), 0.001f);
	BOOST_CHECK_VECTOR_CLOSE(v[3u], sf::Vector2f(0.257f, 1.669f), 0.001f);
	
	for (auto i = 4u; i < 10u; ++i) {
		BOOST_CHECK_VECTOR_CLOSE(v[i], sf::Vector2f(1.f, 1.f), 0.001f); // center which means to skip rest of the circle
	}
}

BOOST_AUTO_TEST_SUITE_END()

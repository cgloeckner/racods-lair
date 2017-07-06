#include <vector>
#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>
#include <testsuite/singleton.hpp>

#include <core/algorithm.hpp>

std::vector<sf::Time> testChunked(
	sf::Time const& elapsed, sf::Time const& frametime) {
	std::vector<sf::Time> times;
	core::updateChunked(
		[&](sf::Time const& t) { times.push_back(t); }, elapsed, frametime);
	return times;
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(algorithm_test)

BOOST_AUTO_TEST_CASE(
	updateChunked_chunks_2_seconds_to_4_parts_of_500_milliseconds) {
	auto delta = sf::milliseconds(500);
	auto times = testChunked(sf::seconds(2.f), delta);
	BOOST_REQUIRE_EQUAL(4u, times.size());
	BOOST_CHECK_TIME_EQUAL(delta, times[0]);
	BOOST_CHECK_TIME_EQUAL(delta, times[3]);
}

BOOST_AUTO_TEST_CASE(
	updateChunked_chunks_2_seconds_to_5_parts_of_450_or_200_milliseconds) {
	auto delta = sf::milliseconds(450);
	auto times = testChunked(sf::seconds(2.f), delta);
	BOOST_REQUIRE_EQUAL(5u, times.size());
	BOOST_CHECK_TIME_EQUAL(delta, times[0]);
	BOOST_CHECK_TIME_EQUAL(delta, times[3]);
	BOOST_CHECK_TIME_EQUAL(sf::milliseconds(200), times[4]);
}

BOOST_AUTO_TEST_CASE(
	updateChunked_chunks_100_seconds_to_1_part_of_200_milliseconds) {
	auto times = testChunked(sf::milliseconds(100), sf::milliseconds(200));
	BOOST_REQUIRE_EQUAL(1u, times.size());
	BOOST_CHECK_TIME_EQUAL(sf::milliseconds(100), times[0]);
}

BOOST_AUTO_TEST_CASE(clockwise_rotated_north_vector_equals_northeast_vector) {
	sf::Vector2i n{0, -1};
	auto ne = core::rotate(n, true);
	BOOST_CHECK_VECTOR_EQUAL(ne, sf::Vector2i(1, -1));
}

BOOST_AUTO_TEST_CASE(
	counterclockwise_rotated_north_vector_equals_northwest_vector) {
	sf::Vector2i n{0, -1};
	auto nw = core::rotate(n, false);
	BOOST_CHECK_VECTOR_EQUAL(nw, sf::Vector2i(-1, -1));
}

BOOST_AUTO_TEST_CASE(
	rotate_vector_clockwise_is_inverse_to_counter_clockwise_rotation) {
	sf::Vector2i dir;
	for (dir.y = -1; dir.y <= 1; ++dir.y) {
		for (dir.x = -1; dir.x <= 1; ++dir.x) {
			if (dir.x == 0 && dir.y == 0) {
				continue;
			}
			auto clockwise = core::rotate(dir, true);
			auto counterwise = core::rotate(clockwise, false);
			BOOST_CHECK_VECTOR_EQUAL(counterwise, dir);
		}
	}
}

BOOST_AUTO_TEST_SUITE_END()

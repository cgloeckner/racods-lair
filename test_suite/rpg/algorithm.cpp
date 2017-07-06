#include <vector>
#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>
#include <testsuite/singleton.hpp>

#include <rpg/algorithm.hpp>

BOOST_AUTO_TEST_SUITE(algorithm_test)

BOOST_AUTO_TEST_CASE(extrapolate_with_zero_exponent_returns_factor) {
	auto value = rpg::extrapolate(100.f, 1.1f, 0.f);
	
	BOOST_CHECK_EQUAL(value, 100u);
}

BOOST_AUTO_TEST_CASE(extrapolate_with_one_exponent_returns_zero) {
	auto value = rpg::extrapolate(100.f, 1.1f, 1.f);
	
	BOOST_CHECK_EQUAL(value, 110u);
}

BOOST_AUTO_TEST_SUITE_END()

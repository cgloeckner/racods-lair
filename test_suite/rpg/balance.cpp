#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>
#include <testsuite/singleton.hpp>

#include <rpg/balance.hpp>

BOOST_AUTO_TEST_SUITE(balance_test)

BOOST_AUTO_TEST_CASE(level_zero_player_has_zero_exp) {
	auto exp = rpg::getNextExp(0u);
	
	BOOST_CHECK_EQUAL(exp, 0u);
}

BOOST_AUTO_TEST_CASE(level_one_player_has_n_exp) {
	auto exp = rpg::getNextExp(1u);
	
	BOOST_CHECK_EQUAL(exp, 100u);
}

BOOST_AUTO_TEST_CASE(level_two_player_has_some_more_exp) {
	auto exp = rpg::getNextExp(2u);
	
	BOOST_CHECK_EQUAL(exp, 400u);
}

BOOST_AUTO_TEST_CASE(exp_is_always_greater_zero) {
	auto exp = rpg::getExpGain(1, 0);
	BOOST_CHECK_GT(exp, 0u);
}

BOOST_AUTO_TEST_SUITE_END()

#include <vector>
#include <boost/test/unit_test.hpp>

#include <utils/assert.hpp>

void only_positive(int i) { ASSERT(i > 0); }

// ----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(assert_test)

BOOST_AUTO_TEST_CASE(assert_require) {
	BOOST_REQUIRE_ASSERT(only_positive(-1));
}

BOOST_AUTO_TEST_CASE(assert_check) { BOOST_CHECK_ASSERT(only_positive(-1)); }

BOOST_AUTO_TEST_CASE(assert_require_none) {
	BOOST_REQUIRE_NO_ASSERT(only_positive(2));
}

BOOST_AUTO_TEST_CASE(assert_check_none) {
	BOOST_CHECK_NO_ASSERT(only_positive(2));
}

BOOST_AUTO_TEST_CASE(assert_multiple_check) {
	BOOST_CHECK_ASSERT(only_positive(-3));
	BOOST_CHECK_ASSERT(only_positive(0));
	BOOST_CHECK_ASSERT(only_positive(-2));
}

BOOST_AUTO_TEST_CASE(assert_multiple_check_and_check_none) {
	BOOST_CHECK_ASSERT(only_positive(-3));
	BOOST_CHECK_NO_ASSERT(only_positive(2));
	BOOST_CHECK_ASSERT(only_positive(-2));
	BOOST_CHECK_NO_ASSERT(only_positive(3));
	BOOST_CHECK_NO_ASSERT(only_positive(4));
	BOOST_CHECK_ASSERT(only_positive(-10));
	BOOST_CHECK_ASSERT(only_positive(-1));
}

BOOST_AUTO_TEST_SUITE_END()

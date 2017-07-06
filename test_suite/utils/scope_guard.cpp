#include <vector>
#include <boost/test/unit_test.hpp>

#include <utils/scope_guard.hpp>

BOOST_AUTO_TEST_SUITE(scope_guard_test)

BOOST_AUTO_TEST_CASE(scope_guard_enter_func_on_construction) {
	int i{0};
	utils::ScopeGuard guard([&]() { ++i; }, [&]() {});

	BOOST_CHECK_EQUAL(i, 1);
}

BOOST_AUTO_TEST_CASE(scope_guard_exit_func_on_destruction) {
	int i{0};
	{
		utils::ScopeGuard guard([&]() {}, [&]() { ++i; });
	}
	BOOST_CHECK_EQUAL(i, 1);
}

BOOST_AUTO_TEST_SUITE_END()

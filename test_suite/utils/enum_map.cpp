#include <boost/test/unit_test.hpp>

#include <utils/enum_map.hpp>

namespace test {
enum class Example { Red = 0u, Green, Apple, Mercedes, Tank, Demo };
}

using namespace test;

SET_ENUM_LIMITS(Example::Red, Example::Demo);

// ----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(EnumMap_test)

BOOST_AUTO_TEST_CASE(EnumMap_construction_allocates_all_elements) {
	utils::EnumMap<Example, std::string> map;

	BOOST_CHECK_EQUAL(6u, map.size());
}

BOOST_AUTO_TEST_CASE(EnumMap_construction_initializes_all_elements) {
	utils::EnumMap<Example, std::string> map;

	BOOST_CHECK_EQUAL("", map[Example::Apple]);
	BOOST_CHECK_EQUAL("", map[Example::Demo]);
}

BOOST_AUTO_TEST_CASE(EnumMap_get_set_consistently) {
	utils::EnumMap<Example, std::string> map;

	map[Example::Apple] = "Foo bar";
	BOOST_CHECK_EQUAL("Foo bar", map[Example::Apple]);
}

BOOST_AUTO_TEST_CASE(EnumMap_non_const_iteration_order) {
	utils::EnumMap<Example, std::string> map;
	map[Example::Red] = "Foo bar";
	map[Example::Green] = "baz lol";

	auto b = std::begin(map);
	BOOST_CHECK(b->first == Example::Red);
	BOOST_CHECK_EQUAL(b->second, "Foo bar");

	++b;
	BOOST_CHECK(b->first == Example::Green);
	BOOST_CHECK_EQUAL(b->second, "baz lol");
}

BOOST_AUTO_TEST_CASE(EnumMap_const_iteration_order) {
	utils::EnumMap<Example, std::string> map;
	map[Example::Red] = "Foo bar";
	map[Example::Green] = "baz lol";

	auto b = std::begin(
		const_cast<utils::EnumMap<Example, std::string> const &>(map));
	BOOST_CHECK(b->first == Example::Red);
	BOOST_CHECK_EQUAL(b->second, "Foo bar");

	++b;
	BOOST_CHECK(b->first == Example::Green);
	BOOST_CHECK_EQUAL(b->second, "baz lol");
}

BOOST_AUTO_TEST_CASE(EnumMap_can_be_copied) {
	utils::EnumMap<Example, std::string> map;
	map[Example::Red] = "Foo bar";
	map[Example::Green] = "baz lol";

	auto copy = map;
	for (auto const &pair : map) {
		BOOST_CHECK_EQUAL(copy[pair.first], pair.second);
	}
}

BOOST_AUTO_TEST_CASE(EnumMaps_are_equal_if_all_pairs_equal_pairwise) {
	utils::EnumMap<Example, std::string> lhs, rhs;
	lhs[Example::Red] = "Foo bar";
	lhs[Example::Green] = "baz lol";
	rhs[Example::Red] = "Foo bar";
	rhs[Example::Green] = "baz lol";

	BOOST_CHECK(lhs == rhs);
}

BOOST_AUTO_TEST_CASE(EnumMaps_are_equal_if_at_least_one_pair_differs) {
	utils::EnumMap<Example, std::string> lhs, rhs;
	lhs[Example::Red] = "Foo bar";
	lhs[Example::Green] = "baz lol";
	rhs[Example::Red] = "Foo bar";
	rhs[Example::Green] = "rofl";

	BOOST_CHECK(lhs != rhs);
}

BOOST_AUTO_TEST_SUITE_END()

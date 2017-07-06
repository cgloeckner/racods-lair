#include <sstream>
#include <boost/test/unit_test.hpp>

#include <utils/enum_utils.hpp>

namespace test {
ENUM(OS, Linux, (Linux)(Apple)(Windows))

// static test against ambigous from_string() declaration
ENUM(Room, Doors, (Doors)(Tables)(Windows)(Walls))

ENUM(Atomic, Foo, (Foo))
}

using namespace test;

ENUM_STREAM(OS)
SET_ENUM_LIMITS(OS::Linux, OS::Windows)
SET_ENUM_LIMITS(Atomic::Foo, Atomic::Foo)

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(EnumUtils_test)

BOOST_AUTO_TEST_CASE(EnumUtils_default_value_is_first_value) {
	auto os = default_value<OS>();
	auto room = default_value<Room>();
	BOOST_CHECK_EQUAL(
		static_cast<std::size_t>(OS::Linux), static_cast<std::size_t>(os));
	BOOST_CHECK_EQUAL(
		static_cast<std::size_t>(Room::Doors), static_cast<std::size_t>(room));
}

BOOST_AUTO_TEST_CASE(EnumUtils_valid_to_string) {
	auto o = to_string(OS::Linux);
	auto r = to_string(Room::Walls);
	BOOST_CHECK_EQUAL("Linux", o);
	BOOST_CHECK_EQUAL("Walls", r);
}

BOOST_AUTO_TEST_CASE(EnumUtils_compatibly_to_ostream_operator) {
	std::stringstream s;
	s << OS::Linux;
	BOOST_CHECK_EQUAL(s.str(), "Linux");
}

BOOST_AUTO_TEST_CASE(EnumUtils_invalid_to_string) {
	BOOST_CHECK_THROW(to_string(static_cast<OS>(Room::Walls)), std::bad_cast);
	BOOST_CHECK_THROW(to_string(static_cast<Room>(7)), std::bad_cast);
}

BOOST_AUTO_TEST_CASE(EnumUtils_valid_from_string) {
	auto os = from_string<OS>("Windows");
	auto room = from_string<Room>("Windows");
	BOOST_CHECK_EQUAL(
		static_cast<std::size_t>(OS::Windows), static_cast<std::size_t>(os));
	BOOST_CHECK_EQUAL(static_cast<std::size_t>(Room::Windows),
		static_cast<std::size_t>(room));
}

BOOST_AUTO_TEST_CASE(EnumUtils_invalid_from_string) {
	BOOST_CHECK_THROW(from_string<OS>("BDS"), std::bad_cast);
	BOOST_CHECK_THROW(from_string<Room>("Linux"), std::bad_cast);
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(EnumRange_begin_returns_min_value) {
	utils::EnumRange<OS> range;
	BOOST_CHECK_EQUAL(static_cast<std::size_t>(OS::Linux),
		static_cast<std::size_t>(*range.begin()));
}

BOOST_AUTO_TEST_CASE(EnumRange_end_returns_successor_of_max_value) {
	utils::EnumRange<OS> range;
	BOOST_CHECK_EQUAL(static_cast<std::size_t>(OS::Windows) + 1,
		static_cast<std::size_t>(*range.end()));
}

BOOST_AUTO_TEST_CASE(
	EnumRange_iterations_delivers_all_values_in_correct_order) {
	utils::EnumRange<OS> range;
	auto i = range.begin();
	BOOST_CHECK_EQUAL(
		static_cast<std::size_t>(OS::Linux), static_cast<std::size_t>(*i));
	++i;
	BOOST_CHECK_EQUAL(
		static_cast<std::size_t>(OS::Apple), static_cast<std::size_t>(*i));
	++i;
	BOOST_CHECK_EQUAL(
		static_cast<std::size_t>(OS::Windows), static_cast<std::size_t>(*i));
	++i;
	BOOST_CHECK_EQUAL(static_cast<std::size_t>(OS::Windows) + 1,
		static_cast<std::size_t>(*i));
}

BOOST_AUTO_TEST_CASE(EnumRange_does_not_continue_iteration_after_end) {
	utils::EnumRange<OS> range;
	auto i = range.end();
	BOOST_CHECK_EQUAL(static_cast<std::size_t>(OS::Windows) + 1,
		static_cast<std::size_t>(*i));
	++i;
	BOOST_CHECK_EQUAL(static_cast<std::size_t>(OS::Windows) + 1,
		static_cast<std::size_t>(*i));
}

BOOST_AUTO_TEST_CASE(EnumRange_works_for_atomic_enums) {
	utils::EnumRange<Atomic> range;
	auto b = range.begin();
	auto e = range.end();
	BOOST_CHECK(b != e);
	++b;
	BOOST_CHECK(b == e);
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(getEnumCount_works_for_typical_enums) {
	BOOST_CHECK_EQUAL(utils::getEnumCount<OS>(), 3u);
}

BOOST_AUTO_TEST_CASE(getEnumCount_works_for_atomic_enum) {
	BOOST_CHECK_EQUAL(utils::getEnumCount<Atomic>(), 1u);
}

BOOST_AUTO_TEST_SUITE_END()

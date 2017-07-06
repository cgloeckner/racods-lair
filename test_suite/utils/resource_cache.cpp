#include <boost/test/unit_test.hpp>

#include <utils/resource_cache.hpp>

struct FooRes {
	std::string str;

	bool loadFromFile(std::string const& name) {
		str = name;
		return true;
	}
};

struct BarRes {
	std::size_t num;

	bool loadFromFile(std::string const& name) {
		num = name.size();
		return true;
	}
};

using MyCache = utils::MultiResourceCache<FooRes, BarRes>;

// ----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(ResourceCache_test)

BOOST_AUTO_TEST_CASE(ResourceCache_acquire_foo_reference) {
	MyCache cache;
	auto& my_foo = cache.get<FooRes>("hello world");

	BOOST_CHECK_EQUAL("hello world", my_foo.str);
	BOOST_CHECK_EQUAL(&my_foo, &cache.get<FooRes>("hello world"));
}

BOOST_AUTO_TEST_CASE(ResourceCache_acquire_bar_reference) {
	MyCache cache;
	auto& my_bar = cache.get<BarRes>("cya world");

	BOOST_CHECK_EQUAL(9u, my_bar.num);
	BOOST_CHECK_EQUAL(&my_bar, &cache.get<BarRes>("cya world"));
}

BOOST_AUTO_TEST_CASE(ResourceCache_acquire_both_references) {
	MyCache cache;
	auto& my_foo = cache.get<FooRes>("hello world");
	auto& my_bar = cache.get<BarRes>("cya world");
	auto& again = cache.get<FooRes>("hello world");
	auto& another = cache.get<BarRes>("cya world");

	BOOST_CHECK_EQUAL("hello world", my_foo.str);
	BOOST_CHECK_EQUAL(9u, my_bar.num);
	BOOST_CHECK_EQUAL(&my_foo, &again);
	BOOST_CHECK_EQUAL(&my_bar, &another);
}

BOOST_AUTO_TEST_SUITE_END()

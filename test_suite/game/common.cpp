#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>

/*
#include <game/common.hpp>

struct FooSystem {
	std::size_t n;

	FooSystem()
		: n{0u} {
	}

	void update(sf::Time const & elapsed) {
		++n;
	}
};

struct BarSystem {
	std::size_t n;

	BarSystem()
		: n{0u} {
	}

	void update(sf::Time const & elapsed) {
		++n;
	}
};

BOOST_AUTO_TEST_SUITE(common_test)

BOOST_AUTO_TEST_CASE(update_multiple_systems) {
	FooSystem foo;
	BarSystem bar;

	game::EntityUpdater<FooSystem, BarSystem> updater{foo, bar};
	updater.update(sf::milliseconds(30));
	updater.update(sf::milliseconds(20));
	updater.update(sf::milliseconds(27));
	updater.update(sf::milliseconds(53));

	BOOST_CHECK_EQUAL(foo.n, 4u);
	BOOST_CHECK_EQUAL(bar.n, 4u);

	updater.getTotal<FooSystem>();
	updater.getTotal<BarSystem>();
}

BOOST_AUTO_TEST_SUITE_END()
*/

#include <vector>
#include <boost/test/unit_test.hpp>

#include <utils/delay_system.hpp>

BOOST_AUTO_TEST_SUITE(delay_test)

BOOST_AUTO_TEST_CASE(values_are_forwarded_as_soon_as_they_are_ready) {
	utils::DelaySystem<std::string> delay;

	delay.push("first", sf::milliseconds(100));
	delay.push("second", sf::milliseconds(50));
	delay.push("fifth", sf::milliseconds(250));
	delay.push("fourth", sf::milliseconds(110));
	delay.push("third", sf::milliseconds(60));
	delay.push("sixth", sf::milliseconds(500));
	delay.push("seventh", sf::milliseconds(600));
	delay(sf::milliseconds(100));

	BOOST_REQUIRE_EQUAL(delay.ready.size(), 3u);
	BOOST_CHECK_EQUAL(delay.ready[0], "first");
	BOOST_CHECK_EQUAL(delay.ready[1], "second");
	BOOST_CHECK_EQUAL(delay.ready[2], "third");
	delay.ready.clear();

	delay(sf::milliseconds(20));
	BOOST_REQUIRE_EQUAL(delay.ready.size(), 1u);
	BOOST_CHECK_EQUAL(delay.ready[0], "fourth");
	delay.ready.clear();

	delay(sf::milliseconds(100));
	BOOST_REQUIRE(delay.ready.empty());

	delay(sf::milliseconds(100));
	BOOST_REQUIRE_EQUAL(delay.ready.size(), 1u);
	BOOST_CHECK_EQUAL(delay.ready[0], "fifth");
	delay.ready.clear();

	delay(sf::milliseconds(180));
	BOOST_REQUIRE_EQUAL(delay.ready.size(), 1u);
	BOOST_CHECK_EQUAL(delay.ready[0], "sixth");

	delay.reset();
	delay(sf::milliseconds(120));
	BOOST_REQUIRE(delay.ready.empty());
}

BOOST_AUTO_TEST_SUITE_END()

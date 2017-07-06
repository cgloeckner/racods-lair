#include <boost/test/unit_test.hpp>

#include <utils/binary_utils.hpp>

enum class BinaryTest { Foo = 0u, Bar, Baz };

SET_ENUM_LIMITS(BinaryTest::Foo, BinaryTest::Baz);

BOOST_AUTO_TEST_SUITE(xmlBinary_test)

BOOST_AUTO_TEST_CASE(dump_and_parse_vector_traverses_each_element) {
	sf::Packet stream;
	std::vector<std::string> values = {"hello", "world", "i'm a", "test"};

	utils::dump(stream, values, [&](std::string const & s) {
		stream << s;
	});

	sf::Packet packet;
	packet.append(stream.getData(), stream.getDataSize());
	std::vector<std::string> loaded;
	utils::parse(packet, loaded, [&](std::string& s) {
		packet >> s;
	});

	BOOST_REQUIRE_EQUAL(loaded.size(), values.size());
	BOOST_CHECK(loaded == values);
}

BOOST_AUTO_TEST_CASE(dump_and_parse_empty_vector_works) {
	sf::Packet stream;
	std::vector<std::string> values;

	utils::dump(stream, values, [&](std::string const & s) {
		stream << s;
	});

	sf::Packet packet;
	packet.append(stream.getData(), stream.getDataSize());
	std::vector<std::string> loaded;
	utils::parse(packet, loaded, [&](std::string& s) {
		packet >> s;
	});

	BOOST_REQUIRE_EQUAL(loaded.size(), values.size());
	BOOST_CHECK(loaded == values);
}

BOOST_AUTO_TEST_CASE(dump_and_parse_array_traverses_each_element) {
	sf::Packet stream;
	std::array<int, 3> values;
	values[0] = 3;
	values[1] = 1;
	values[2] = 5;

	utils::dump(stream, values, [&](int i) {
		stream << i;
	});

	sf::Packet packet;
	packet.append(stream.getData(), stream.getDataSize());
	decltype(values) loaded;
	utils::parse(packet, loaded, [&](int& i) {
		packet >> i;
	});

	BOOST_CHECK(loaded == values);
}

BOOST_AUTO_TEST_CASE(dump_and_parse_enumMap_traverses_each_element) {
	sf::Packet stream;
	utils::EnumMap<BinaryTest, float> values;
	values[BinaryTest::Foo] = 3.14f;
	values[BinaryTest::Bar] = 5.53f;
	values[BinaryTest::Baz] = -0.33f;

	utils::dump(stream, values, [&](float f) {
		stream << f;
	});

	sf::Packet packet;
	packet.append(stream.getData(), stream.getDataSize());
	decltype(values) loaded;
	utils::parse(packet, loaded, [&](float& f) {
		packet >> f;
	});

	BOOST_CHECK(loaded == values);
}

BOOST_AUTO_TEST_SUITE_END()

#include <boost/test/unit_test.hpp>

#include <map>
#include <utils/xml_utils.hpp>

BOOST_AUTO_TEST_SUITE(xmlUtils_test)

BOOST_AUTO_TEST_CASE(dump_and_parse_vector_traverses_each_element) {
	utils::ptree_type ptree;
	std::vector<std::string> values = {"hello", "world", "i'm a", "test"};

	// dump to ptree
	utils::dump_vector(ptree, "foo", "item", values,
		[](utils::ptree_type& child, std::string const& str) {
			child.put("<xmlattr>.str", str);
		});

	// parse from ptree (twice!)
	std::vector<std::string> array;
	utils::parse_vector(ptree, "foo", "item", array,
		[](utils::ptree_type const& child, std::string& str) {
			str = child.get<std::string>("<xmlattr>.str");
		});
	utils::parse_vector(ptree, "foo", "item", array,
		[](utils::ptree_type const& child, std::string& str) {
			str = child.get<std::string>("<xmlattr>.str");
		});

	// expect same arrays
	BOOST_CHECK(array == values);
}

BOOST_AUTO_TEST_CASE(dump_and_parse_vector_traverses_each_element_without_root_tag) {
	utils::ptree_type ptree;
	std::vector<std::string> values = {"hello", "world", "i'm a", "test"};

	// dump to ptree
	utils::dump_vector(ptree, "item", values,
		[](utils::ptree_type& child, std::string const& str) {
			child.put("<xmlattr>.str", str);
		});

	// parse from ptree (twice!)
	std::vector<std::string> array;
	utils::parse_vector(ptree, "item", array,
		[](utils::ptree_type const& child, std::string& str) {
			str = child.get<std::string>("<xmlattr>.str");
		});
	utils::parse_vector(ptree, "item", array,
		[](utils::ptree_type const& child, std::string& str) {
			str = child.get<std::string>("<xmlattr>.str");
		});

	// expect same arrays
	BOOST_CHECK(array == values);
}

BOOST_AUTO_TEST_CASE(dump_vector_creates_correct_tag_names) {
	utils::ptree_type ptree;
	std::vector<std::string> values = {"hello", "world", "i'm a", "test"};

	// dump to ptree
	utils::dump_vector(ptree, "foo", "item", values,
		[](utils::ptree_type& child, std::string const& str) {
			child.put("<xmlattr>.str", str);
		});

	// expect children named "foo" and elements named "item"
	BOOST_REQUIRE_NO_THROW(ptree.get_child("foo"));
	auto const& container = ptree.get_child("foo");
	std::size_t i = 0;
	for (auto const& child : container) {
		if (child.first != "item") {
			BOOST_FAIL("Children #" + std::to_string(i) + " not named 'item'");
		}
		++i;
	}
}

BOOST_AUTO_TEST_CASE(dump_vector_creates_correct_tag_names_without_root_tag) {
	utils::ptree_type ptree;
	std::vector<std::string> values = {"hello", "world", "i'm a", "test"};

	// dump to ptree
	utils::dump_vector(ptree, "item", values,
		[](utils::ptree_type& child, std::string const& str) {
			child.put("<xmlattr>.str", str);
		});

	// expect elements named "item"
	auto const& container = ptree;
	std::size_t i = 0;
	for (auto const& child : container) {
		if (child.first != "item") {
			BOOST_FAIL("Children #" + std::to_string(i) + " not named 'item'");
		}
		++i;
	}
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(dump_and_parse_array_traverses_each_element) {
	utils::ptree_type ptree;
	std::array<std::string, 4u> values = {{"hello", "world", "i'm a", "test"}};

	// dump to ptree
	utils::dump_array(ptree, "foo", "item", values,
		[](utils::ptree_type& child, std::string const& str) {
			child.put("<xmlattr>.str", str);
		});

	// parse from ptree (twice!)
	std::array<std::string, 4u> array;
	utils::parse_array(ptree, "foo", "item", array,
		[](utils::ptree_type const& child, std::string& str) {
			str = child.get<std::string>("<xmlattr>.str");
		});
	utils::parse_array(ptree, "foo", "item", array,
		[](utils::ptree_type const& child, std::string& str) {
			str = child.get<std::string>("<xmlattr>.str");
		});

	// expect same arrays
	BOOST_CHECK(array == values);
}

BOOST_AUTO_TEST_CASE(dump_array_creates_correct_tag_names) {
	utils::ptree_type ptree;
	std::array<std::string, 4u> values = {{"hello", "world", "i'm a", "test"}};

	// dump to ptree
	utils::dump_array(ptree, "foo", "item", values,
		[](utils::ptree_type& child, std::string const& str) {
			child.put("<xmlattr>.str", str);
		});

	// expect children named "foo" and elements named "item"
	BOOST_REQUIRE_NO_THROW(ptree.get_child("foo"));
	auto const& container = ptree.get_child("foo");
	std::size_t i = 0;
	for (auto const& child : container) {
		if (child.first != "item") {
			BOOST_FAIL("Children #" + std::to_string(i) + " not named 'item'");
		}
		++i;
	}
	BOOST_REQUIRE_EQUAL(i, 4u);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(dump_and_parse_map_traverses_each_element) {
	utils::ptree_type ptree;
	std::map<float, int> values;
	values[3.14] = 3;
	values[5.34] = 5;

	// dump to ptree
	utils::dump_map(ptree, "foo", "item", values,
		[](utils::ptree_type& child, float key, int value) {
			child.put("<xmlattr>.f", key);
			child.put("<xmlattr>.i", value);
		});
	
	// parse from ptree
	std::map<float, int> map;
	map[0.3] = 0; // should be dropped during parsing
	utils::parse_map(ptree, "foo", "item", map,
		[](utils::ptree_type const& child, float& key, int& value) {
			key = child.get<float>("<xmlattr>.f");
			value = child.get<int>("<xmlattr>.i");
		});
	
	// expect same arrays
	BOOST_CHECK(map == values);
}

BOOST_AUTO_TEST_CASE(dump_map_creates_correct_tag_names) {
	utils::ptree_type ptree;
	std::map<float, int> values;
	values[3.14] = 3;
	values[5.34] = 5;

	// dump to ptree
	utils::dump_map(ptree, "foo", "item", values,
		[](utils::ptree_type& child, float key, int value) {
			child.put("<xmlattr>.f", key);
			child.put("<xmlattr>.i", value);
		});

	// expect children named "foo" and elements named "item"
	BOOST_REQUIRE_NO_THROW(ptree.get_child("foo"));
	auto const& container = ptree.get_child("foo");
	std::size_t i = 0;
	for (auto const& child : container) {
		if (child.first != "item") {
			BOOST_FAIL("Children #" + std::to_string(i) + " not named 'item'");
		}
		++i;
	}
	BOOST_REQUIRE_EQUAL(i, 2u);
}

BOOST_AUTO_TEST_SUITE_END()

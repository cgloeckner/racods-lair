#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>

#include <utils/assert.hpp>
#include <utils/lua_utils.hpp>

BOOST_AUTO_TEST_SUITE(lua_test)

BOOST_AUTO_TEST_CASE(globals_can_be_get) {
	utils::Script script;
	BOOST_REQUIRE(script.loadFromMemory(
		"value = false\nfoo = function()\n\tvalue = true\nend\n"));
	BOOST_CHECK(script.get<bool>("value") == false);
}

BOOST_AUTO_TEST_CASE(function_can_be_called) {
	utils::Script script;
	BOOST_REQUIRE(script.loadFromMemory(
		"value = false\nfoo = function()\n\tvalue = true\nend\n"));
	script("foo");
	BOOST_CHECK(script.get<bool>("value") == true);
}

BOOST_AUTO_TEST_CASE(function_can_be_called_with_args) {
	utils::Script script;
	std::string text{"hello world"};
	float number{3.14};
	BOOST_REQUIRE(script.loadFromMemory(
		"text = ''\nnumber = 0\nfoo = function(s, f)\n\ttext = s\n\tnumber = f\nend\n"));
	script("foo", text, number);
	BOOST_CHECK_EQUAL(script.get<std::string>("text"), text);
	BOOST_CHECK_CLOSE(script.get<float>("number"), number, 0.0001f);
}

BOOST_AUTO_TEST_CASE(globals_can_be_set) {
	utils::Script script;
	BOOST_REQUIRE(script.loadFromMemory(
		"value = false\nfoo = function()\n\tvalue = true\nend\n"));
	script.set("value", true);
	BOOST_CHECK(script.get<bool>("value") == true);
}

BOOST_AUTO_TEST_SUITE_END()
